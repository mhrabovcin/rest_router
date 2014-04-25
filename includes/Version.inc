<?php

/**
 * @file
 * API version parser
 */

abstract class RestRouterVersion {

  public static $version_regex = '(?P<major>\d+)(?:\.(?P<minor>\d+))?';

  /**
   * Processed request
   *
   * @var RestRouterRequest
   */
  protected $request;

  /**
   * Plugin settings
   *
   * @var array
   */
  protected $settings = array();

  function __construct(RestRouterRequest $request, $settings = array()) {
    $this->request = $request;
    // Allow endpoint override configuration
    $this->settings = $settings + $this->settings;
  }

  /**
   * Return current plugin settings.
   */
  public function getSettings() {
    return $this->settings;
  }

  /**
   * Get version that plugin can detect from request
   *
   * @return
   *   Version number if detected, otherwise NULL
   */
  abstract public function version();

  /**
   * Alter path to contain version
   *
   * @param $path
   *   API path
   * @param $options
   *   URL options
   */
  abstract public function url(&$path, &$options);
}

class RestRouterVersionPath extends RestRouterVersion {

  public function version() {
    $matches = array();
    $path = $this->request->getPath();
    if (preg_match('~^v' . self::$version_regex . '/(.*)$~', $path, $matches)) {
      $version = $matches['major'] . '.' . (!empty($matches['minor']) ? $matches['minor'] : '0');
      $path = preg_replace('~^v' . self::$version_regex . '/(.*)$~', '\3', $path);
      $this->request->setPath($path);
    }

    return !empty($version) ? $version : NULL;
  }

  public function url(&$path, &$options) {
    $path = 'v' . $this->request->getVersion() . '/' . $path;
  }
}

class RestRouterVersionQuery extends RestRouterVersion {

  protected $settings = array('key' => 'v');

  public function version() {
    $matches = array();
    if (($version = $this->request->get($this->settings['key']))) {
      if (preg_match('~' . self::$version_regex . '~', $version, $matches)) {
        $version = $matches['major'] . '.' . (!empty($matches['minor']) ? $matches['minor'] : '0');
        return $version;
      }
    }

    return NULL;
  }

  public function url(&$path, &$options) {
    $options['query'][$this->settings['key']] = $this->request->getVersion();
  }
}
