#ifndef CORAL_HPP_
#define CORAL_HPP_

#include "request.hpp"
#include "wire.hpp"

/** @file coral */
namespace inaccel {

/** @brief Submit a request to the FPGA resource manager.
 *
 *  @details The %request argument should be in a format compliant with the
 *  specifications demonstated in request docs. This method exits as soon as
 *  the request is transmitted to the resource manager and wait() should be
 *  later called to wait for its completion.
 *
 *  @param req The request to be submitted to the FPGA resource manager
 *  @returns The session id to later wait the request.
*/
session submit(request& req);

/** Block until the completion of a request corresponding to the
 *  supplied session id. This method should be invoked after a
 *  submit() to wait for the completion of the request. An
 *  invocation of this method without prior request submission will
 *  cause a runtime exception.
 *
 *  @param wire The id corresponding to the request on which we
 *  should wait.
*/
void wait(session wire);

}	// namespace inaccel

#endif // CORAL_HPP_
