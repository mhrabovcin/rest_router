<?php

/**
 * @file
 * Custom exception
 */

class RestRouterException extends Exception {

  public $code;

  public $message;

  public $data;

  function __construct($code, $message = NULL, $data = NULL) {
    $this->code    = $code;
    $this->message = $message;
    $this->data    = $data;
  }
}
