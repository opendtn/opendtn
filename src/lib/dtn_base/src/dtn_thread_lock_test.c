/***

Copyright   2018       German Aerospace Center DLR e.V.,
                        German Space Operations Center (GSOC)

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.

    This file is part of the openvocs project. http://openvocs.org

***/ /**

     \file               dtn_thread_lock_tests.c
     \author             Michael J. Beer, DLR/GSOC <michael.beer@dlr.de>
     \date               2018-01-19

     \ingroup            empty

 **/
/*---------------------------------------------------------------------------*/

#include "../include/dtn_time.h"
#include "../include/dtn_utils.h"
#include "../include/testrun.h"
#include "dtn_thread_lock.c"

/*---------------------------------------------------------------------------*/

/**
 * Retrieve usecs passed since 'usec'.
 * To retrieve the absolute time, use USECS_SINCE(0)
 */
#define USECS_SINCE(usec)                                                      \
    llabs((int64_t)dtn_time_get_current_time_usecs() - (int64_t)usec)

/*****************************************************************************
 * HELPERS
 *****************************************************************************/

struct lock_result {
    uint64_t usecs_passed_for_try_lock;
    bool could_lock;
};

/*---------------------------------------------------------------------------*/

void *lock_locker(void *arg) {

    dtn_thread_lock *lock = (dtn_thread_lock *)arg;
    struct lock_result *r = calloc(1, sizeof(struct lock_result));

    uint64_t usecs = USECS_SINCE(0);
    r->could_lock = dtn_thread_lock_try_lock(lock);
    usecs = USECS_SINCE(usecs);
    r->usecs_passed_for_try_lock = usecs;

    if (r->could_lock)
        dtn_thread_lock_unlock(lock);

    return r;
}

/*---------------------------------------------------------------------------*/

typedef enum {

    SHOULD_HAVE_BEEN_LOCKED,
    SHOULD_NOT_HAVE_BEEN_LOCKED

} lock_result_t;

/*---------------------------------------------------------------------------*/

bool check_with_thread(dtn_thread_lock *lock, lock_result_t expected,
                       uint64_t min_usecs, uint64_t max_usecs) {

    void *retval = 0;
    pthread_t id;

    if (0 != pthread_create(&id, 0, lock_locker, lock))
        return false;
    if (0 != pthread_join(id, &retval))
        return false;

    struct lock_result *lr = (struct lock_result *)retval;

    bool success = false;
    switch (expected) {

    case SHOULD_HAVE_BEEN_LOCKED:

        success = lr->could_lock;
        break;

    case SHOULD_NOT_HAVE_BEEN_LOCKED:

        success = !lr->could_lock;
        break;

    default:

        testrun(!"INVALID CASE");
    };

    success = success && (lr->usecs_passed_for_try_lock >= min_usecs);
    success = success && (lr->usecs_passed_for_try_lock <= max_usecs);

    free(lr);

    return success;
}

/*****************************************************************************
 * HELPERS - WAIT / NOTIFY
 *****************************************************************************/

struct wait_and_set_params {

    bool set_flag;
    dtn_thread_lock *lock;
    uint64_t usecs_passed;
};

/*---------------------------------------------------------------------------*/

void *wait_and_set(void *arg) {

    struct wait_and_set_params *p = (struct wait_and_set_params *)arg;

    DTN_ASSERT(p->lock);

    p->set_flag = false;

    uint64_t usecs = USECS_SINCE(0);

    DTN_ASSERT(dtn_thread_lock_try_lock(p->lock));

    if (dtn_thread_lock_wait(p->lock)) {

        p->set_flag = true;
    }

    DTN_ASSERT(dtn_thread_lock_unlock(p->lock));

    usecs = USECS_SINCE(usecs);

    p->usecs_passed = usecs;

    return p;
}

/******************************************************************************
 *                                   TESTS
 ******************************************************************************/

int test_dtn_thread_lock_init() {

    testrun(!dtn_thread_lock_init(0, 0));

    dtn_thread_lock lock = {0};
    testrun(!dtn_thread_lock_init(&lock, 0));

    testrun(dtn_thread_lock_init(&lock, 17));
    testrun(17 * 1000 == lock.timeout.tv_nsec);
    testrun(0 == lock.timeout.tv_sec);

    dtn_thread_lock_clear(&lock);

    testrun(dtn_thread_lock_init(&lock, 21 * 1000 * 1000 + 17));
    testrun(17 * 1000 == lock.timeout.tv_nsec);
    testrun(21 == lock.timeout.tv_sec);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_thread_lock_clear() {

    testrun(!dtn_thread_lock_clear(0));

    dtn_thread_lock lock = {0};
    testrun(dtn_thread_lock_init(&lock, 31));
    testrun(dtn_thread_lock_clear(&lock));

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_thread_lock_try_lock() {

    const uint64_t timeout_ms = 1000;
    const uint64_t max_deviation_ms = 400;

    testrun(!dtn_thread_lock_try_lock(0));

    dtn_thread_lock lock = {0};
    testrun(dtn_thread_lock_init(&lock, timeout_ms * 1000));

    testrun(check_with_thread(&lock, SHOULD_HAVE_BEEN_LOCKED, 0,
                              timeout_ms * 1000));

    uint64_t usecs = USECS_SINCE(0);
    testrun(dtn_thread_lock_try_lock(&lock));
    usecs = USECS_SINCE(usecs);
    testrun(usecs < timeout_ms * 1000);

    usecs = USECS_SINCE(0);
    testrun(!dtn_thread_lock_try_lock(&lock));
    usecs = USECS_SINCE(usecs);

    /*
     *      Got some failure here during some tests.
     *      do we need to change the values?
     */
    fprintf(stderr, "usecs: %" PRIu64 " | min %" PRIu64 " max %" PRIu64 "\n",
            usecs, (timeout_ms - max_deviation_ms) * 1000,
            (timeout_ms + max_deviation_ms) * 1000);

    testrun(usecs < (timeout_ms + max_deviation_ms) * 1000);
    testrun(usecs > (timeout_ms - max_deviation_ms) * 1000);

    /* Should also block other threads */
    testrun(check_with_thread(&lock, SHOULD_NOT_HAVE_BEEN_LOCKED,
                              timeout_ms * 1000, 2 * timeout_ms * 1000));

    dtn_thread_lock_unlock(&lock);

    dtn_thread_lock_clear(&lock);

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_thread_lock_unlock() {

    testrun(!dtn_thread_lock_unlock(0));

    dtn_thread_lock lock = {0};
    testrun(dtn_thread_lock_init(&lock, 100 * 1000));

    testrun(dtn_thread_lock_try_lock(&lock));
    testrun(!dtn_thread_lock_try_lock(&lock));
    testrun(dtn_thread_lock_unlock(&lock));
    testrun(dtn_thread_lock_try_lock(&lock));

    // Should also work between threads...
    testrun(check_with_thread(&lock, SHOULD_NOT_HAVE_BEEN_LOCKED, 100 * 1000,
                              20 * 100 * 1000));

    /* Increase the timeout value to 1 sec */
    testrun(dtn_thread_lock_unlock(&lock));
    testrun(dtn_thread_lock_clear(&lock));
    testrun(dtn_thread_lock_init(&lock, 1000 * 1000));

    testrun(dtn_thread_lock_try_lock(&lock));

    /* Should block ... */
    void *retval = 0;
    pthread_t id;

    testrun(0 == pthread_create(&id, 0, lock_locker, &lock));

    /* Now wait 100msecs, then unlock */
    struct timespec time_to_wait = {

        .tv_nsec = 100 * 1000 * 1000,

    };
    testrun(0 == nanosleep(&time_to_wait, 0));
    testrun(dtn_thread_lock_unlock(&lock));

    testrun(0 == pthread_join(id, &retval));

    struct lock_result *lr = (struct lock_result *)retval;

    testrun(lr->could_lock);

    testrun(lr->usecs_passed_for_try_lock >= .8 * 100 * 1000);
    testrun(lr->usecs_passed_for_try_lock <= 2 * 100 * 1000);

    free(lr);

    testrun(dtn_thread_lock_unlock(&lock));

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_thread_lock_wait() {

    testrun(!dtn_thread_lock_wait(0));

    struct wait_and_set_params params = {0};

    dtn_thread_lock lock;
    /* We dont want the timeout to trigger */
    testrun(dtn_thread_lock_init(&lock, 1 * 1000 * 1000));

    params.lock = &lock;

    pthread_t id;

    testrun(0 == pthread_create(&id, 0, wait_and_set, &params));

    struct timespec time_to_wait = {
        .tv_nsec = 500 * 1000 * 1000,
    };

    nanosleep(&time_to_wait, 0);

    /* Thread should still be blocked */

    testrun(dtn_thread_lock_notify(&lock));
    /* Now it should have been woken up and terminate immediately ...*/

    void *retval = 0;
    testrun(0 == pthread_join(id, retval));

    /* Should last less than 500msecs until we return ... */

    testrun(params.set_flag);
    testrun(1.2 * 500 * 1000 > params.usecs_passed);

    /* Check again for timeout on wait() */
    testrun(0 == pthread_create(&id, 0, wait_and_set, &params));

    testrun(0 == pthread_join(id, retval));
    testrun(!params.set_flag);

    testrun(0.8 * 1000 * 1000 < params.usecs_passed);
    testrun(1.2 * 1000 * 1000 > params.usecs_passed);

    testrun(dtn_thread_lock_clear(&lock));

    return testrun_log_success();
}

/*---------------------------------------------------------------------------*/

int test_dtn_thread_lock_notify() {

    /* Fucked up solution - but in order to test wait() properly, we have to
     * rely upon notify(), and the other way round -
     * there is absolutely no advantage in falling back to bare posix
     * signalling just to decouple these two tests - IMHO.
     */
    return test_dtn_thread_lock_wait();
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST CLUSTER                                                    #CLUSTER
 *
 *      ------------------------------------------------------------------------
 */

int all_tests() {

    testrun_init();
    testrun_test(test_dtn_thread_lock_init);
    testrun_test(test_dtn_thread_lock_clear);
    testrun_test(test_dtn_thread_lock_try_lock);
    testrun_test(test_dtn_thread_lock_unlock);
    testrun_test(test_dtn_thread_lock_wait);
    testrun_test(test_dtn_thread_lock_notify);

    return testrun_counter;
}

/*
 *      ------------------------------------------------------------------------
 *
 *      TEST EXECUTION                                                  #EXEC
 *
 *      ------------------------------------------------------------------------
 */

testrun_run(all_tests);