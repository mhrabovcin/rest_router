<?php
use Drupal\rest_router\response\StructuredErrorResponse;

/**
 * @file
 */

abstract class RestRouterApiEndpoint {

  /**
   * Processed request
   *
   * @var RestRouterRequest
   */
  protected $request;


  /**
   * Constructor
   */
  public function __construct(RestRouterRequest $request) {
    $this->request = $request;
  }

  /**
   * Create response object with provided code and data.
   *
   * @param int $code
   *   Valid HTTP response code.
   *
   * @param mixed $data
   *   Response data.
   *
   * @return \RestRouterResponse
   */
  protected function response($code, $data = NULL) {
    return new RestRouterResponse($code, $data);
  }

  /**
   * Create error response object with provided code, message and data.
   *
   * @param int $code
   *   Valid HTTP response code.
   *
   * @param string $message
   *   Additional response message.
   *
   * @param mixed $data
   *   Response data.
   *
   * @return \RestRouterErrorResponse
   */
  protected function errorResponse($code, $message, $data = NULL) {
    return new RestRouterErrorResponse($code, $message, $data);
  }

  /**
   * Create structured error response.
   *
   * @param int $code
   *   Valid HTTP response code.
   *
   * @param string $message
   *   Additional response message.
   *
   * @return \Drupal\rest_router\response\StructuredErrorResponse
   */
  protected function structuredErrorResponse($code, $message) {
    return new StructuredErrorResponse($code, $message);
  }

  /**
   * Create redirect response object with provided path and code.
   *
   * Path shouldn't contain api prefix or version just internal API
   * path which will be formatted to full path.
   *
   * @param string $path
   *   Internal API path without prefixes. I.e.
   *   If path is available on /my-api/v2.0/test/method $path arg should be
   *   test/method only.
   *
   * @param int $code
   *   Redirect HTTP code, 301 by default is used.
   *
   * @return \RestRouterRedirectResponse
   */
  protected function redirectResponse($path, $code = 301) {
    return new RestRouterRedirectResponse($path, $code);
  }

  /**
   * Run operations before a endpoint method is called.
   *
   * @param string $method
   *   Name of request method being invoked.
   * @param array $args
   *   Array of arguments passed to method.
   */
  public function beforeRequest($method, array $args = array()) {}

  /**
   * Run operations after a endpoint method is called.
   *
   * @param string $method
   *   Name of request method being invoked.
   * @param array $args
   *   Array of arguments passed to method.
   * @param mixed $response_value
   *   Response from the method, could be mixed value or \RestRouterResponse.
   */
  public function afterRequest($method, array $args = array(), &$response_value) {}
}
