<?php

/**
 * @file
 * API documentation for Rest Router module
 */

/**
 * Specifies REST endpoints provided by module
 *
 * @return
 *   Array of endpoints provided by module where key is endpoint machine name.
 */
function hook_rest_endpoints() {
  return array(
    // Key specifies endpoint machine name
    'nspi_api' => array(
      // Path where should be endpoint mapped
      'path' => 'api',
      // User friendly name of endpoint
      'name' => 'NSPI API',
      // List of versions provided by API. Key of array represents name of version
      'versions' => array(
        // API version
        '1.0' => array(
          // Class of router, must inherit RestRouterApiRouter
          'router' => 'NspiApiRouterV1',
          // Callbacks class, must inherit RestRouterEndpoint
          'class' => 'NspiApiV1'
        ),
        '2.0' => array('router' => 'NspiApiRouterV1', 'class' => 'NspiApiV2'),
      ),
      // Default version of API if none provided. If this will be NULL and none
      // version will be parsed from HTTP request router will generated 404 response.
      'default version' => '1.0',
      // List of supported authenticatin plugins
      'auth' => array(
        // Key is machine name of plugin and value is array which represents
        // configuration.
        'oauth' => array(
          'type' => '2legged',
          'context' => 'nspi_api'
        ),
      ),
      // Version parser plugins. They will be executed in order they are specified
      // in this array. Each plugin accepts configuration.
      'version' => array('path' => array(), 'query' => array()),
      // Supported request formats. Client must specify request format via request
      // header. I.e. Content-Type: application/json
      'request' => array('json'),
      // Supported response formats. Client must specify response format via header
      // Accept: application/json
      'response' => array('json'),
    ),
  );
}

/**
 * Allows to change definition of endpoints
 *
 * @param $endpoints
 *   Endpoints provided by modules
 */
function hook_rest_endpoints_alter(&$endpoints) {
  if (isset($endpoint['nsp_api'])) {
    $endpoint['nsp_api']['path'] = 'mypath';
  }
}

/**
 * Allows to change processed request
 *
 * @param $request
 *   RestRouterRequest
 */
function hook_rest_router_request_alter($request) {
  if ($request->get('key') == 'value') {
    $request->setPath('new/path');
  }
}

/**
 * Each router definition must inherit RestRouterApiRouter class and implement
 * router() method.
 */
class CustomRestRouterV1 extends RestRouterApiRouter {

  /**
   * Returns list of available routes that API provides.
   *
   * This is very similar to core hook_menu but doesn't creates items in {menu}
   * mysql table neither creates links. It has additional support for HTTP request
   * methods and routing to object methods.
   *
   * @return
   *   Array of routes
   */
  public function routes() {
    $items = array();

    // As key we specify METHOD:defined/path for alter purposes
    $items[self::HTTP_POST . ':my/path/%api::loader_callback'] = array(
      // Path can contain normal loader functions i.e. %node which will translate
      // to node_load or API class specific functions which can be defined as
      // in example: %api::loader_callback. This definition would call method of initialzed
      // endpoint object $endpoint->loader_callback([arg])
      'path' => 'subscriptions/%api::loader_callback',
      // Page callback can contain normal php function i.e. node_view or API method
      // in format api::method
      'page callback' => 'api::page_callback',
      // This works exactly as normal hook_menu implementation
      'page arguments' => array(1),
      // Can contain normal function or objec method
      'access callback' => 'api::access_callback',
      // Works as hook_menu
      'access arguments' => array(1),
      // Specifies HTTP method to which callback respons
      'http method' => self::HTTP_GET,
    );

    return $items;
  }
}

class CustomRestApiV1 extends RestRouterApiEndpoint {
  /**
   * Executed by router.
   *
   * @param $arg
   */
  public function page_callback($arg) {
    // POST data can't be mapped to callback arguments but each API class
    // is initialized with request object which contains DATA that aren't passed
    // via URL.

    // To access get data
    $this->request->get('KEY');

    // To access POST/PUT/DELETE parsed data
    $all_data = $this->request->data();
    $specific = $this->request->data('user_key');

    // Each method should return raw PHP data
    return array('key' => 'value');
  }
}
