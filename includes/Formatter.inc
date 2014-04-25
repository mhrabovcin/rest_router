<?php

/**
 * @file
 * Response formatters
 */

class RestRouterFormatter {

  /**
   * Request
   *
   * @var RestRouterRequest
   */
  protected $request;

  /**
   * Response
   *
   * @var RestRouterResponse
   */
  protected $response;

  public function __construct(RestRouterRequest $request, RestRouterResponse $response) {
    $this->request  = $request;
    $this->response = $response;
    $accept = $this->request->headers('ACCEPT');
    $this->formatter = $this->getFormatter($accept);
  }

  /**
   * Retrieve formatter request
   *
   * @return
   *   RestRouterRequest
   */
  public function getRequest() {
    return $this->request;
  }

  /**
   * Set formatter request
   *
   * @param RestRouterRequest $request
   *   New request
   */
  public function setRequest(RestRouterRequest $request) {
    $this->request = $request;
    $accept = $this->request->headers('ACCEPT');
    $this->formatter = $this->getFormatter($accept);
  }

  /**
   * Get response
   *
   * @return
   *   RestRouterResponse
   */
  public function getResponse() {
    return $this->response;
  }

  /**
   * Set new response
   *
   * @param RestRouterResponse $response
   *   New response
   */
  public function setResponse(RestRouterResponse $response) {
    $this->response = $response;
  }

  /**
   * Prepare formatter for response creation.
   */
  public function prepare() {
    if (!empty($this->formatter)) {
      $this->response->addHeader('Content-Type', $this->formatter['content_type']);
    }
    else {
      $this->response->setCode(406);
    }
  }

  /**
   * Format response data
   *
   * @return
   *   Formatted response string
   */
  public function format() {
    if ($this->formatter && $this->response->data()) {
      return call_user_func_array(array($this, $this->formatter['callback']), array($this->response->data()));
    }
    return NULL;
  }

  /**
   * Retrieve response formatter
   *
   * @param $accept
   *   Accept mime from request header
   * @return
   *   Formatter info
   */
  private function getFormatter($accept) {
    $endpoint = $this->request->endpoint();
    $formatters = self::formatters();

    foreach ($formatters as $name => $formatter) {
      // Check if formatter is enabled for endpoint and if it matches accept MIME
      if (in_array($name, $endpoint['response']) && in_array($accept, $formatter['accept'])) {
        return $formatter;
      }
    }

    // No formatter found
    return NULL;
  }

  /**
   * Format JSON data callback.
   *
   * @param $data
   *   Data to be formatted
   * @return
   *   JSON encoded string
   */
  protected function formatJSON($data) {
    return json_encode($data);
  }

  /**
   * Get list of available response formatters.
   *
   * @return
   *   Array of formatters.
   */
  public static function formatters() {
    return array(
      'json' => array(
        'callback' => 'formatJSON',
        'content_type' => 'application/json',
        'accept' => array('application/json'),
      ),
    );
  }
}
