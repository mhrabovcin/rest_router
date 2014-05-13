REST Router
===========

Module that allows to create multiple REST endpoints defined in code. No UI is
included in this module. Key features:

  - lightweight
  - supports versions of API with plugins
  - authentication plugins
  - routing to objects

### Using router

To create new rest endpoint module needs to implement `hook_rest_endpoints`. Hook documentation is available in `rest_router.api.php`.

##### Custom responses

Object or function callbacks called by rest_router module can return repsonse in different data formats. Scalar responses are always converted to array

```php
    function my_callback() {
        return "value";
    }
```

Will become in response data

```php
    array("value")
```

If other than `200 OK` response is required function can return one of response defined response objects.

- `new RestRouterResponse($code, $data = NULL)`
- `RestRouterErrorResponse($code, $message, $data = NULL)`
- `RestRouterRedirectResponse($path, $code)`

Each response has helper method that can be used from inherited class.

**Examples**

```php
    public function myMethod() {
        // Custom response by initializing class
        return new RestRouterResponse(204);

        // Response by internal helper
        return $this->errorResponse(400, "Missing param [1]");

        // Redirect resposne exmaple
        return $this->redirectResponse('method/2');
    }
```

### Ideas

- Allow API version definition to override 'auth', 'request', 'response' settings.

### Tests

Unit tests are for classes. To run them

    drush test-run "Rest Router Unit"

Web test cases can be run by

    drush test-run "Rest Router"
