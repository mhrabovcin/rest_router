<?php

/**
 * @file
 * Rest Router Test API implementation
 */

class RestRouterTestRouterV1 extends RestRouterApiRouter {
  public function routes() {
    $items = array();

    $items[self::HTTP_GET . ':test'] = array(
      'path' => 'test',
      'page callback' => 'api::test',
      'access callback' => 1
    );

    $items[self::HTTP_GET . ':denied'] = array(
      'path' => 'denied',
      'page callback' => 'api::test',
      'access callback' => 'api::access_false',
    );

    $items[self::HTTP_GET . ':approved'] = array(
      'path' => 'approved',
      'page callback' => 'api::test',
      'access callback' => 'api::access_true',
    );

    $items[self::HTTP_GET . ':arg/%'] = array(
      'path' => 'arg/%',
      'page callback' => 'api::test_arg',
      'page arguments' => array(1),
      'access callback' => 1
    );

    $items[self::HTTP_GET . ':arg-transform/%api::arg_transform'] = array(
      'path' => 'arg-transform/%api::arg_transform',
      'page callback' => 'api::test_arg',
      'page arguments' => array(1),
      'access callback' => 1
    );

    $items[self::HTTP_GET . ':response/%'] = array(
      'path' => 'response/%',
      'page callback' => 'api::test_response',
      'page arguments' => array(1),
      'access callback' => 1
    );

    $items[self::HTTP_GET . ':access-user'] = array(
      'path' => 'access-user',
      'page callback' => 'api::test',
      'access callback' => 'api::access_user',
    );

    return $items;
  }
}

class RestRouterTestRouterV2 extends RestRouterTestRouterV1 {
  public function routes() {
    $items = parent::routes();
    $items[self::HTTP_GET . ':denied']['access callback'] = 'api::access_true';
    unset($items[self::HTTP_GET . ':approved']);
    return $items;
  }
}

class RestRouterTestApiV1 extends RestRouterApiEndpoint {
  public function test() {
    return "test";
  }

  public function test_arg($arg) {
    return $arg;
  }

  public function access_false() {
    return FALSE;
  }

  public function access_true() {
    return TRUE;
  }

  public function access_user() {
    global $user;
    return !empty($user->uid);
  }

  public function arg_transform($arg) {
    return $arg . '_transformed';
  }

  public function test_response($type) {
    // Test common responses
    $map = array(
      'ok' => 200,
      'not_found' => 404,
      'access_denied' => 401,
      'internal_error' => 500,
    );
    if (isset($map[$type])) {
      return new RestRouterResponse($map[$type], $type);
    }

    if ($type == 'redirect') {
      return new RestRouterRedirectResponse('response/redirected', 302);
    }
  }
}

class RestRouterTestApiV2 extends RestRouterTestApiV1 {
  public function test() {
    return "testv2";
  }
}
