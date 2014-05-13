<?php

/**
 * @file
 * Api Router
 */

abstract class RestRouterApiRouter {

  /**
   * HTTP methods list
   */
  const HTTP_GET      = 'GET';
  const HTTP_POST     = 'POST';
  const HTTP_PUT      = 'PUT';
  const HTTP_DELETE   = 'DELETE';
  const HTTP_OPTIONS  = 'OPTIONS';
  const HTTP_PATCH    = 'PATCH';

  /**
   * Endpoint definition
   */
  protected $endpoint;

  /**
   * Request holding state
   *
   * @var RestRouterRequest
   */
  protected $request;

  /**
   * API version
   */
  protected $version;

  /**
   * Custom menu mask array
   *
   * @var array
   */
  protected $mask;

  /**
   * Store original menu masks
   *
   * @var array
   */
  private $originalMasks;

  /**
   * Constructor
   *
   * @param $request
   *   Processed HTTP request
   */
  public function __construct(RestRouterRequest $request) {
    $this->request     = $request;
    $this->endpoint    = $request->endpoint();
    $this->version     = $this->request->getVersion();
    $this->class       = $this->endpoint['versions'][$this->version]['class'];
    $this->api         = new $this->class($request);
  }

  /**
   * Get available routes for API
   *
   * @return
   *   Array of hook_menu callbacks
   */
  abstract public function routes();

  /**
   * Get processed request
   */
  public function request() {
    return $this->request;
  }

  /**
   * Process HTTP request
   */
  public function process() {
    $item = $this->menuItem($this->request->getPath(), $this->request->method());
    $output = NULL;
    if (!empty($item['access'])) {
      if ($item['include file']) {
        require_once DRUPAL_ROOT . '/' . $item['include file'];
      }
      $page_callback_result = call_user_func_array($item['page callback'], $item['page arguments']);
      // Callback can return instance of RestRouterResponse class
      if (is_object($page_callback_result) && is_a($page_callback_result, 'RestRouterResponse')) {
        $output = $page_callback_result;
      }
      else {
        // If normal value is returned create RestRouterResponse
        if (!is_array($page_callback_result) && !is_object($page_callback_result)) {
          $page_callback_result = array($page_callback_result);
        }
        $output = new RestRouterResponse(200, $page_callback_result);
      }
    }
    elseif (!empty($item)) {
      $output = new RestRouterErrorResponse(401, "Access denied");
    }
    else {
      $output = new RestRouterErrorResponse(404, '');
    }

    return $output;
  }

  /**
   * Get router menu item with access checking and args loading
   */
  protected function menuItem($path, $method) {
    $original_map = arg(NULL, $path);
    $item = $this->menuMatchItem($path, $method);
    if ($item) {
      $map = _menu_translate($item, $original_map);
      $item['original_map'] = $original_map;
      if ($map === FALSE) {
        return FALSE;
      }
      if ($item['access']) {
        $item['map'] = $map;
        $item['page arguments'] = menu_unserialize($item['page arguments'], $map);
        $item['theme arguments'] = menu_unserialize($item['theme arguments'], $map);
      }
    }
    else {
      throw new RestRouterException(404, '');
    }

    return $item;
  }

  /**
   * Retrieve menu item that is matching path + method
   */
  protected function menuMatchItem($path, $method) {
    $routes = $this->getRoutes();
    $parts = explode('/', $path);

    // To get correct list of ancestors we need to override menu masks
    // variable with custom map from our routes. After ancestors are retrieved
    // reset menu_masks variable to Drupal default.
    $this->overrideMenuMask($this->mask);
    $ancestors = menu_get_ancestors($parts);
    $this->resetMenuMask();

    uasort($routes, 'RestRouterApiRouter::compareMenuItems');
    foreach ($ancestors as $ancestor) {
      $key = $method . '/' . $ancestor;
      if (isset($routes[$key])) {
        return $routes[$key];
      }
    }
  }

  /**
   * Temporarily override menu_masks array with custom routes configuration.
   *
   * @param array $mask
   *   Drupal menu router bitmask
   */
  public function overrideMenuMask($mask = array()) {
    global $conf;
    $this->originalMasks = $conf['menu_masks'];
    $conf['menu_masks'] = $mask;
  }

  /**
   * Reset router mask overrides.
   */
  public function resetMenuMask() {
    global $conf;
    $conf['menu_masks'] = $this->originalMasks;
  }

  /**
   * Compare two menu items callback.
   *
   * @param array $a
   *   First item to compare
   *
   * @param array $b
   *   Second item to compare
   *
   * @return int
   *   Comparison result: -1, 0, 1
   */
  static function compareMenuItems($a, $b) {
    return version_compare($a['_fit'], $b['_fit']) * -1;
  }

  /**
   * Retrieve processed routes
   *
   * @return
   *   Array
   */
  protected function getRoutes() {
    $routes = $this->routes();
    drupal_alter('rest_routes', $routes, $this->endpoint);
    list($menu, $mask) = $this->buildRouter($routes);
    // Store custom ancestor mask
    $this->mask = $mask;
    return $menu;
  }

  protected function buildRouter($callbacks) {
    // First pass: separate callbacks from paths, making paths ready for
    // matching. Calculate fitness, and fill some default values.
    $menu = array();
    $masks = array();
    $sort = array();
    foreach ($callbacks as $id => $item) {
      $path = $item['path'];
      $load_functions = array();
      $to_arg_functions = array();
      $fit = 0;
      $move = FALSE;

      $parts = explode('/', $path, MENU_MAX_PARTS);
      $number_parts = count($parts);
      // We store the highest index of parts here to save some work in the fit
      // calculation loop.
      $slashes = $number_parts - 1;
      // Extract load and to_arg functions.
      foreach ($parts as $k => $part) {
        $match = FALSE;
        // Look for wildcards in the form allowed to be used in PHP functions,
        // because we are using these to construct the load function names.
        if (preg_match('/^%(|' . DRUPAL_PHP_FUNCTION_PATTERN . '|api\:\:' . DRUPAL_PHP_FUNCTION_PATTERN . ')$/', $part, $matches)) {
          if (empty($matches[1])) {
            $match = TRUE;
            $load_functions[$k] = NULL;
          }
          else {
            if (function_exists($matches[1] . '_to_arg')) {
              $to_arg_functions[$k] = $matches[1] . '_to_arg';
              $load_functions[$k] = NULL;
              $match = TRUE;
            }
            if (function_exists($matches[1] . '_load')) {
              $function = $matches[1] . '_load';
              // Create an array of arguments that will be passed to the _load
              // function when this menu path is checked, if 'load arguments'
              // exists.
              $load_functions[$k] = isset($item['load arguments']) ? array($function => $item['load arguments']) : $function;
              $match = TRUE;
            }
            elseif(($method = $this->get_object_method($matches[1])) && method_exists($this->class, $method)) {
              // We have local object method that should be called as arg loader
              $load_functions[$k] = array('rest_router_loader_callback' => array($this->api, $method));
              $match = TRUE;
            }
          }
        }
        if ($match) {
          $parts[$k] = '%';
        }
        else {
          $fit |=  1 << ($slashes - $k);
        }
      }
      if ($fit) {
        $move = TRUE;
      }
      else {
        // If there is no %, it fits maximally.
        $fit = (1 << $number_parts) - 1;
      }
      $masks[$fit] = 1;
      $item['_load_functions'] = $load_functions;
      $item['to_arg_functions'] = empty($to_arg_functions) ? '' : serialize($to_arg_functions);
      $item += array(
        'title' => '',
        'weight' => 0,
        'type' => MENU_NORMAL_ITEM,
        'module' => '',
        '_number_parts' => $number_parts,
        'number_parts' => $number_parts,
        'title_callback' => 't',
        '_parts' => $parts,
        '_fit' => $fit,
      );
      $item += array(
        '_visible' => (bool) ($item['type'] & MENU_VISIBLE_IN_BREADCRUMB),
        '_tab' => (bool) ($item['type'] & MENU_IS_LOCAL_TASK),
      );

      if (!isset($item['access callback']) && isset($item['access arguments'])) {
        // Default callback.
        $item['access callback'] = 'user_access';
      }
      if (!isset($item['access callback']) || empty($item['page callback'])) {
        $item['access callback'] = 0;
      }
      if (is_bool($item['access callback'])) {
        $item['access callback'] = intval($item['access callback']);
      }

      $item['load_functions'] = empty($item['_load_functions']) ? '' : serialize($item['_load_functions']);
      $item['path'] = $path = implode('/', $parts);
      $item += array(
        'access arguments' => array(),
        'access callback' => '',
        'page arguments' => array(),
        'page callback' => '',
        'delivery callback' => '',
        'title arguments' => array(),
        'title callback' => 't',
        'theme arguments' => array(),
        'theme callback' => '',
        'description' => '',
        'position' => '',
        'context' => 0,
        'tab_parent' => '',
        'tab_root' => $path,
        'file' => '',
        'file path' => '',
        'include file' => '',
        'http method' => 'GET',
      );

      // Calculate out the file to be included for each callback, if any.
      if ($item['file']) {
        $file_path = $item['file path'] ? $item['file path'] : drupal_get_path('module', $item['module']);
        $item['include file'] = $file_path . '/' . $item['file'];
      }

      // Check if callback is part of object
      if ($callback_method = $this->get_object_method($item['page callback'])) {
        $item['page callback'] = 'rest_router_page_callback';
        array_unshift($item['page arguments'], $this->api, $callback_method);
      }

      if ($access_method = $this->get_object_method($item['access callback'])) {
        $item['access callback'] = 'rest_router_access_callback';
        array_unshift($item['access arguments'], $this->api, $access_method);
      }

      $item['access_callback'] = $item['access callback'];
      $item['access_arguments'] = serialize($item['access arguments']);
      $item['page arguments'] = serialize($item['page arguments']);
      $item['theme arguments'] = serialize($item['theme arguments']);

      if ($move) {
        $new_path = $item['http method'] . '/' . implode('/', $item['_parts']);
        $menu[$new_path] = $item;
        $sort[$new_path] = $number_parts;
      }
      else {
        $path = $item['http method'] . '/' . $path;
        $menu[$path] = $item;
        $sort[$path] = $number_parts;
      }
    }
    array_multisort($sort, SORT_NUMERIC, $menu);

    // Sort the masks so they are in order of descending fit.
    $masks = array_keys($masks);
    rsort($masks);

    return array($menu, $masks);
  }

  protected function get_object_method($name) {
    if (strpos($name, 'api::') === 0 && ($method = substr($name, 5))) {
      return $method;
    }

    return NULL;
  }
}
