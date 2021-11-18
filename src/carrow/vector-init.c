
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#include "vector.h"
#include "vector-init.h"
#include "vector-parse-format.h"
#include "status.h"

int arrow_vector_init(struct ArrowVector* vector, struct ArrowSchema* schema,
                      struct ArrowArray* array, struct ArrowStatus* status) {
  arrow_status_reset(status);

  arrow_vector_set_schema(vector, schema, status);
  RETURN_IF_NOT_OK(status);

  arrow_vector_set_array(vector, array, status);
  RETURN_IF_NOT_OK(status);

  return 0;
}

int arrow_vector_set_schema(struct ArrowVector* vector, struct ArrowSchema* schema,
                            struct ArrowStatus* status) {
  arrow_status_reset(status);
  if (vector == NULL) {
    arrow_status_set_error(status, EINVAL, "`vector` is NULL");
    RETURN_IF_NOT_OK(status);
  }

  // reset values
  vector->schema = NULL;
  vector->array = NULL;
  vector->type = ARROW_TYPE_MAX_ID;
  vector->data_buffer_type = ARROW_TYPE_MAX_ID;
  vector->args = "";
  vector->n_buffers = 0;
  vector->element_size_bytes = -1;

  vector->has_validity_buffer = 0;
  vector->offset_buffer_id = -1;
  vector->large_offset_buffer_id = -1;
  vector->union_type_buffer_id = -1;
  vector->data_buffer_id = -1;

  arrow_status_reset(status);

  if (schema != NULL) {
    if (schema->release == NULL) {
      arrow_status_set_error(status, EINVAL, "`schema` is released");
      RETURN_IF_NOT_OK(status);
    }

    arrow_vector_parse_format(vector, schema->format, status);
    RETURN_IF_NOT_OK(status);
  }

  vector->schema = schema;
  return 0;
}

int arrow_vector_set_array(struct ArrowVector* vector, struct ArrowArray* array,
                           struct ArrowStatus* status) {
  arrow_status_reset(status);

  if (vector == NULL) {
    arrow_status_set_error(status, EINVAL, "`vector` is NULL");
    RETURN_IF_NOT_OK(status);
  }

  if (array != NULL) {
    if (array->release == NULL) {
      arrow_status_set_error(status, EINVAL, "`array` is released");
      RETURN_IF_NOT_OK(status);
    }

    if (array->n_buffers == vector->n_buffers) {
      vector->has_validity_buffer = 0;
    } else if (array->n_buffers == (vector->n_buffers + 1)) {
      vector->has_validity_buffer = 1;
    } else {
      arrow_status_set_error(
        status, EINVAL,
        "Expected %l or %l buffers in array but found %l",
        vector->n_buffers, vector->n_buffers + 1, array->n_buffers
      );
      RETURN_IF_NOT_OK(status);
     }
  }

  vector->array = array;
  return 0;
}