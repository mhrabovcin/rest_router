<?php

/**
 * @file
 * Response class which specifies output of API
 */

class RestRouterResponse {

  /**
   * Response code
   */
  protected $code;

  /**
   * Response data
   */
  protected $data;

  /**
   * Headers array
   */
  protected $headers;

  /**
   * Request
   *
   * @var RestRouterRequest
   */
  protected $request;

  /**
   * Array HTTP response codes and messages
   */
  protected static $messages = array(
    //Informational 1xx
    100 => '100 Continue',
    101 => '101 Switching Protocols',
    //Successful 2xx
    200 => '200 OK',
    201 => '201 Created',
    202 => '202 Accepted',
    203 => '203 Non-Authoritative Information',
    204 => '204 No Content',
    205 => '205 Reset Content',
    206 => '206 Partial Content',
    //Redirection 3xx
    300 => '300 Multiple Choices',
    301 => '301 Moved Permanently',
    302 => '302 Found',
    303 => '303 See Other',
    304 => '304 Not Modified',
    305 => '305 Use Proxy',
    306 => '306 (Unused)',
    307 => '307 Temporary Redirect',
    //Client Error 4xx
    400 => '400 Bad Request',
    401 => '401 Unauthorized',
    402 => '402 Payment Required',
    403 => '403 Forbidden',
    404 => '404 Not Found',
    405 => '405 Method Not Allowed',
    406 => '406 Not Acceptable',
    407 => '407 Proxy Authentication Required',
    408 => '408 Request Timeout',
    409 => '409 Conflict',
    410 => '410 Gone',
    411 => '411 Length Required',
    412 => '412 Precondition Failed',
    413 => '413 Request Entity Too Large',
    414 => '414 Request-URI Too Long',
    415 => '415 Unsupported Media Type',
    416 => '416 Requested Range Not Satisfiable',
    417 => '417 Expectation Failed',
    422 => '422 Unprocessable Entity',
    423 => '423 Locked',
    //Server Error 5xx
    500 => '500 Internal Server Error',
    501 => '501 Not Implemented',
    502 => '502 Bad Gateway',
    503 => '503 Service Unavailable',
    504 => '504 Gateway Timeout',
    505 => '505 HTTP Version Not Supported'
  );

  function __construct($code, $data = NULL, $headers = array()) {
    $this->code      = $code;
    $this->data      = $data;
    $this->headers   = $headers;
  }

  /**
   * Prepare request for sending
   */
  public function prepare() {
    $this->formatter = new RestRouterFormatter($this->request, $this);
    $this->formatter->prepare();
  }

  /**
   * Add header to response
   *
   * @param $name
   *   Header name
   * @param $value
   *   Header value
   */
  public function addHeader($name, $value) {
    $this->headers[$name] = $value;
  }

  /**
   * Get list of headers
   *
   * @param $name
   *   Name of the header
   * @return
   *   Array of headers of response
   */
  public function headers($name = NULL) {
    if (!empty($name)) {
      return isset($this->headers[$name]) ? $this->headers[$name] : NULL;
    }
    return $this->headers;
  }

  /**
   * Response raw PHP data
   *
   * @return
   *   Array
   */
  public function data() {
    return $this->data;
  }

  /**
   * Set new response data
   *
   * @param $data
   *   Unformatted data
   */
  public function setData($data) {
    $this->data = $data;
  }

  /**
   * Get response code
   *
   * @return
   *   HTTP response code
   */
  public function code() {
    return $this->code;
  }

  /**
   * Sets new response code.
   *
   * @param $code
   *   HTTP code
   */
  public function setCode($code) {
    $this->code = $code;
  }

  /**
   * Get original HTTP request
   *
   * @return
   *   RestRouterRequest
   */
  public function request() {
    return $this->request;
  }

  /**
   * Set request that is processed
   *
   * @param $request
   *   Request - RestRouterRequest
   */
  public function setRequest($request) {
    $this->request = $request;
  }

  /**
   * Get HTTP status response
   *
   * @return
   *   String in format [code] [description], i.e. 200 OK
   */
  public function status() {
    return isset(self::$messages[$this->code]) ? self::$messages[$this->code] : self::$messages[500];
  }

  /**
   * Return formatted body
   * @return [type] [description]
   */
  public function body() {
    if (isset($this->data)) {
      return $this->format();
    }
    else {
      return '';
    }
  }

  /**
   * Format data to requested mime.
   *
   * @param $data
   *   That that needs to be formatted
   * @return
   *   Formatted string
   */
  private function format() {
    return $this->formatter->format();
  }
}

class RestRouterErrorResponse extends RestRouterResponse {

  protected $message;

  function __construct($code, $message, $data = NULL) {
    parent::__construct($code, $data);
    $this->message = $message;
  }

  /**
   * Return additional error message.
   */
  public function getMessage() {
    return $this->message;
  }

  /**
   * Add additional information to status if available error message.
   */
  public function status() {
    if ($this->message) {
      return parent::status() . ': ' . $this->message;
    }
    else {
      return parent::status();
    }
  }
}

class RestRouterRedirectResponse extends RestRouterResponse {

  protected $path;

  function __construct($path, $code = 301) {
    $this->path = $path;
    parent::__construct($code);
  }

  /**
   * Return redirect path.
   */
  public function getPath() {
    return $this->path;
  }

  /**
   * Add redirect location URL
   */
  public function prepare() {
    parent::prepare();
    $this->addHeader('Location', $this->getUrl());
  }

  /**
   * Generate URL
   *
   * @return
   *   URL string
   */
  private function getUrl() {
    return $this->request->url($this->path);
  }
}
