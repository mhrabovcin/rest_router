<?php
/**
 * @file
 * Contains \Drupal\rest_router\response\StructuredErrorResponse.
 */

namespace Drupal\rest_router\response;


/**
 * Extends the error response to provide structured error data.
 *
 * The structure is as follows:
 * {
 *   "errors":
 *     {
 *     "mail":
 *       {
 *         "id":      "mail",
 *         "message": "Invalid e-email address"
 *       },
 *     "name":
 *       {
 *         "id":      "name",
 *         "message": "Duplicate user name"
 *       }
 *    }
 * }
 */
class StructuredErrorResponse extends \RestRouterErrorResponse {

  /**
   * Adds a error message for a given id or field.
   *
   * @param string $id
   *   The id or field on which the error is identified.
   * @param string $message
   *   Error message.
   */
  public function addErrorMessage($id, $message) {
    $this->data['errors'][$id] = array(
      'id' => $id,
      'message' => $message,
    );
  }
}