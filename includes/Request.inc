<?php

/**
 * @file
 * Basic HTTP request state representation
 */

class RestRouterRequest {

  protected $method;

  protected $headers;

  protected $path;

  protected $parser;

  protected $data;

  protected $get;

  protected $server;

  protected $version;

  protected $endpoint;

  /**
   * Constructor
   *
   * @param $endpoint
   *   Endpoint configuration
   * @param $path
   *   Requested query path
   * @param $get
   *   GET array data
   * @param $headers
   *   Array of request headers
   */
  public function __construct($endpoint, $path, $get = NULL, $server = NULL) {
    $this->endpoint = $endpoint;
    $this->path     = $path;
    $this->get      = $get === NULL ? $_GET : $get;
    $this->server   = $server === NULL ? $_SERVER : $server;
    $this->initHeaders();
  }

  /**
   * Initialize header from global request
   */
  private function initHeaders() {
    foreach ($this->server() as $key => $value) {
      if (strpos($key, 'HTTP_') === 0) {
        $this->headers[substr($key, 5)] = $value;
      }
      if (in_array($key, array('CONTENT_LENGTH', 'CONTENT_MD5', 'CONTENT_TYPE'))) {
        $this->headers[$key] = $value;
      }
    }
  }

  /**
   * Prepare request for processing.
   */
  public function prepare() {
    // Create parser
    $this->parser = new RestRouterParser($this);
    $this->data   = $this->parser->parse($this->rawData());
  }

  /**
   * Retrieve key from GET
   *
   * @param $key
   *   Optionally specify request get key
   * @return
   *   Record from GET array or NULL if key is not found
   */
  public function get($key = NULL) {
    if (!empty($key)) {
      return isset($this->get[$key]) ? $this->get[$key] : NULL;
    }
    else {
      return $this->get;
    }
  }

  /**
   * Retrieve key from SERVER
   *
   * @param $key
   *   Optionally specify request get key
   * @return
   *   Record from SERVER array or NULL if key is not found
   */
  public function server($key = NULL) {
    if (!empty($key)) {
      return isset($this->server[$key]) ? $this->server[$key] : NULL;
    }
    else {
      return $this->server;
    }
  }

  /**
   * Request data
   *
   * @param $key
   *   Optionally specific key from DATA
   * @return
   *   Record from data request data or NULL if key is not found
   */
  public function data($key = NULL) {
    if (!empty($key)) {
      return isset($this->data[$key]) ? $this->data[$key] : NULL;
    }
    return $this->data;
  }

  /**
   * Retrieve request content type
   *
   * @return
   *   Content type
   */
  public function contentType() {
    return $this->headers('CONTENT_TYPE');
  }

  /**
   * Retrieve request method
   *
   * @return
   *   Method name i.e. 'GET', 'POST'
   */
  public function method() {
    if (empty($this->method)) {
      $this->method = strtoupper($this->server('REQUEST_METHOD'));
      if ($this->method == 'POST' && ($override = $this->server('HTTP_X_HTTP_METHOD_OVERRIDE'))) {
        $this->method = $override;
      }
      if ($this->method == 'POST' && ($method = $this->get('_method'))) {
        $this->method = $this->get('_method');
        unset($this->get['_method']);
      }
    }

    $this->method = strtoupper($this->method);

    return $this->method;
  }

  /**
   * Retrieve request headers
   *
   * @param $name
   *   Optional header name
   * @return
   *   Header value or headers array
   */
  public function headers($name = NULL) {
    if (!empty($name)) {
      return isset($this->headers[$name]) ? $this->headers[$name] : NULL;
    }
    return $this->headers;
  }

  /**
   * Retrieve raw request data
   *
   * @return
   *   Raw data retrieved from PHP input
   */
  protected function rawData() {
    switch ($this->method()) {
      case 'PUT':
      case 'POST':
      case 'PATCH':
        $handle = fopen('php://input', 'r');
        if ($handle) {
          $content = '';
          while (!feof($handle)) {
            $content .= fread($handle, 8192);
          }
          fclose($handle);
          return $content;
        }
        break;
    }
  }

  /**
   * Return current path
   *
   * @return
   *   Path string
   */
  public function getPath() {
    return $this->path;
  }

  /**
   * Set new path of request
   *
   * @param $path
   *   New path
   */
  public function setPath($path) {
    $this->path = $path;
  }

  /**
   * Get parsed version of requested API
   */
  public function getVersion() {
    return $this->version;
  }

  /**
   * Set the version of requested API
   *
   * @param $version
   *   Formatted version number i.e. '1.0'
   */
  public function setVersion($version) {
    $this->version = $version;
  }

  /**
   * Retrieve endpoint definition.
   *
   * @return
   *   Endpoint that is associated with request
   */
  public function endpoint() {
    return $this->endpoint;
  }

  /**
   * Set version handler for request.
   *
   * @param RestRouterVersion $handler
   */
  public function setVersionHandler(RestRouterVersion $handler) {
    $this->versionHandler = $handler;
  }

  /**
   * Retrieve request version handler
   * @return RestRouterVersion
   */
  public function getVersionHandler() {
    return $this->versionHandler;
  }

  /**
   * Generate URL for provided path.
   *
   * @param $path
   *   Path valid for API context i.e. 'subscriptions/14';
   * @return
   *   Full path
   */
  public function url($path, $options = array()) {
    if ($this->versionHandler) {
      $this->versionHandler->url($path, $options);
    }
    $path = $this->endpoint['path'] . '/' . $path;
    return url($path, $options);
  }
}
