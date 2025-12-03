/***
        ------------------------------------------------------------------------

        Copyright (c) 2019 German Aerospace Center DLR e.V. (GSOC)

        Licensed under the Apache License, Version 2.0 (the "License");
        you may not use this file except in compliance with the License.
        You may obtain a copy of the License at

                http://www.apache.org/licenses/LICENSE-2.0

        Unless required by applicable law or agreed to in writing, software
        distributed under the License is distributed on an "AS IS" BASIS,
        WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
        See the License for the specific language governing permissions and
        limitations under the License.

        This file is part of the openvocs project. https://openvocs.org

        ------------------------------------------------------------------------
*//**
        @file           dtn_node.c
        @author         Markus Töpfer

        @date           2019-11-28

        @ingroup        dtn_node

        @brief          Implementation of a double linked list to
                        inheritage list functionality in any struct.


        ------------------------------------------------------------------------
*/
#include "../include/dtn_node.h"

/*
 *      ------------------------------------------------------------------------
 *
 *      ITERATION
 *
 *      ------------------------------------------------------------------------
 */

void *dtn_node_next(void *data) {

  if (!data)
    return NULL;

  dtn_node *node = (dtn_node *)data;
  return node->next;
}

/*----------------------------------------------------------------------------*/

void *dtn_node_prev(void *data) {

  if (!data)
    return NULL;

  dtn_node *node = (dtn_node *)data;
  return node->prev;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      ITEM ACCESS
 *
 *      ------------------------------------------------------------------------
 */
void *dtn_node_get(void *head, uint64_t pos) {

  if (!head || (pos == 0))
    return NULL;

  dtn_node *next = head;
  uint64_t count = 1;

  while (next) {

    if (count == pos)
      return next;

    next = next->next;
    count++;
  }

  return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_set(void **start, uint64_t pos, void *item) {

  if (!start || !item || (pos == 0))
    goto error;

  dtn_node **head = (dtn_node **)start;
  dtn_node *node = (dtn_node *)item;

  /*
   *      Prevent pushing unclean node.
   */

  if (node->prev || node->next)
    goto error;

  if (!*head) {

    if (pos != 1)
      goto error;

    *head = item;
    return true;
  }

  dtn_node *next = (dtn_node *)*head;
  uint64_t count = 1;

  while (next) {

    if (count == pos) {

      if (next->prev) {
        node->prev = next->prev;
        next->prev->next = node;
      }

      next->prev = node;
      node->next = next;

      if (next == *head)
        *head = node;

      return true;
    }

    count++;

    if ((count == pos) && !next->next) {

      /*
       *      Allow set pos behind last
       *      list item as new list end.
       */

      next->next = node;
      node->prev = next;
      return true;
    }

    next = next->next;
  }

error:
  return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_node_count(void *head) {

  if (!head)
    goto error;

  dtn_node *next = (dtn_node *)head;
  uint64_t count = 1;

  while (next->next) {
    count++;
    next = next->next;
  }

  return count;
error:
  return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_unplug(void **start, void *self) {

  if (!start || !self)
    goto error;

  dtn_node *node = (dtn_node *)self;

  if (self == *start) {
    *start = node->next;
    if (node->next)
      node->next->prev = NULL;
    goto clean;
  }

  if (node->prev)
    node->prev->next = node->next;

  if (node->next)
    node->next->prev = node->prev;

clean:

  node->prev = NULL;
  node->next = NULL;

  return true;
error:
  return false;
}

/*----------------------------------------------------------------------------*/

uint64_t dtn_node_get_position(const void *start, const void *self) {

  if (!start || !self)
    goto error;

  dtn_node *head = (dtn_node *)start;
  dtn_node *node = (dtn_node *)self;

  if (!head || !node)
    goto error;

  dtn_node *next = (dtn_node *)head;
  uint64_t counter = 1;

  while (next) {

    if (next == node)
      return counter;

    counter++;
    next = next->next;
  }

error:
  return 0;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_is_included(const void *start, const void *self) {

  if (!start || !self)
    goto error;

  dtn_node *head = (dtn_node *)start;
  dtn_node *node = (dtn_node *)self;

  dtn_node *next = (dtn_node *)head;

  while (next) {

    if (next == node)
      return true;

    next = next->next;
  }

error:
  return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_remove_if_included(void **start, void *self) {

  if (!start || !self)
    goto error;

  dtn_node *next = (dtn_node *)*start;
  dtn_node *node = (dtn_node *)self;

  while (next) {

    if (next == node)
      return dtn_node_unplug(start, self);

    next = next->next;
  }

  return true;
error:
  return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_push(void **start, void *self) {

  if (!start || !self)
    goto error;

  dtn_node **head = (dtn_node **)start;
  dtn_node *node = (dtn_node *)self;

  /*
   *      Prevent pushing unclean node.
   */

  if (node->prev || node->next)
    goto error;

  if (!*head) {
    *head = self;
    return true;
  }

  dtn_node *next = *head;

  while (next->next)
    next = next->next;

  next->next = node;
  node->prev = next;

  return true;
error:
  return false;
}

/*----------------------------------------------------------------------------*/

void *dtn_node_pop(void **start) {

  if (!start)
    goto error;

  dtn_node **head = (dtn_node **)start;
  if (!*head)
    goto error;

  dtn_node *node = *head;
  dtn_node *next = node->next;

  *head = next;

  if (next)
    next->prev = NULL;

  node->next = NULL;

  return node;
error:
  return NULL;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_push_front(void **start, void *self) {

  if (!start || !self)
    goto error;

  dtn_node *node = (dtn_node *)self;
  dtn_node **head = (dtn_node **)start;

  /*
   *      Prevent pushing unclean node.
   */

  if (node->prev || node->next)
    goto error;

  if (!*head) {
    *head = node;
    return true;
  }

  node->next = *head;
  node->next->prev = node;
  *head = node;

  return true;
error:
  return false;
}

/*----------------------------------------------------------------------------*/

void *dtn_node_pop_first(void **head) { return dtn_node_pop(head); }

/*----------------------------------------------------------------------------*/

bool dtn_node_push_last(void **head, void *node) {

  return dtn_node_push(head, node);
}

/*----------------------------------------------------------------------------*/

void *dtn_node_pop_last(void **head) {

  if (!head || !*head)
    return NULL;

  dtn_node *next = (dtn_node *)*head;

  while (next->next)
    next = next->next;

  if (!dtn_node_unplug(head, next))
    return NULL;

  return next;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_insert_before(void **head, void *n, void *x) {

  if (!head || !n || !x)
    goto error;

  if (n == x)
    goto error;
  if (!*head)
    goto error;

  /*
   *      We perform a check if
   *      x is part of the list,
   *      to prevent unplugging of n.
   */

  dtn_node *next = (dtn_node *)*head;
  bool included = false;

  while (next) {

    if (next == (dtn_node *)x) {
      included = true;
      break;
    }

    next = next->next;
  }

  if (!included)
    return false;

  if (!dtn_node_unplug(head, n))
    goto error;

  dtn_node *node = (dtn_node *)n;
  dtn_node *prev = (dtn_node *)*head;

  if (prev == next) {

    next->prev = node;
    node->next = next;
    node->prev = NULL;

    *head = node;
    return true;
  }

  while (prev) {

    if (prev->next == next) {

      prev->next = node;
      node->next = next;
      next->prev = node;
      node->prev = prev;
      return true;
    }

    prev = prev->next;
  }

  // next not found in list
error:
  return false;
}

/*----------------------------------------------------------------------------*/

bool dtn_node_insert_after(void **head, void *n, void *p) {

  if (!n || !p || !head)
    goto error;
  if (!*head)
    goto error;

  if (n == p)
    goto error;

  dtn_node *prev = (dtn_node *)p;
  dtn_node *node = (dtn_node *)n;
  dtn_node *next = (dtn_node *)*head;

  bool included = false;

  while (next) {

    if (next == prev) {
      included = true;
      break;
    }

    next = next->next;
  }

  if (!included)
    return false;

  if (!dtn_node_unplug(head, node))
    goto error;

  if (prev->next) {

    node->next = prev->next;
    prev->next->prev = node;

  } else {

    node->next = NULL;
  }

  prev->next = node;
  node->prev = prev;

  return true;
error:
  return false;
}