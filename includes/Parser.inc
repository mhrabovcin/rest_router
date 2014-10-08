<?php

/**
 * @file
 * Handles parsing RAW data of incoming request.
 */

class RestRouterParser {

  /**
   * HTTP request
   *
   * @var RestRouterRequest
   */
  protected $request;

  /**
   * Requested content type
   */
  protected $content_type;

  /**
   * Information about parser
   */
  protected $parserInfo;

  public function __construct($request) {
    $this->request      = $request;
    $this->content_type = $request->contentType();
    $this->parserInfo   = $this->getParser($this->content_type);

    if (empty($this->parserInfo)) {
      throw new RestRouterException(406, "Requested response format {$this->content_type} isn't supported.");
    }
  }

  /**
   * Parse raw data from request.
   *
   * @param $data
   *   Raw data content - string
   * @return
   *   Parsed data or NULL
   */
  public function parse($data = NULL) {
    return $this->{$this->parserInfo['callback']}($data);
  }

  /**
   * List of available parsers provided by parser.
   *
   * @return
   *   Array of parser definitions
   */
  public static function parsers() {
    return array(
      'json' => array(
        'callback' => 'parseJSON',
        'mime' => array('application/json')
      ),
    );
  }

  /**
   * Parser JSON data.
   */
  public function parseJSON($data) {
    return json_decode($data, TRUE);
  }


  /**
   * Retrieve parser information based on request.
   *
   * @param $content_type
   *   Request content type
   * @return
   *   Array info about parser
   */
  protected function getParser($content_type) {
    $parsers  = self::parsers();
    $endpoint = $this->request->endpoint();

    $content_types = array_map('trim', explode(';', $content_type));

    foreach ($parsers as $name => $parser) {
      foreach ($content_types as $content_type) {
        if (in_array($name, $endpoint['request']) && in_array($content_type, $parser['mime'])) {
          return $parser;
        }
      }
    }
  }
}
