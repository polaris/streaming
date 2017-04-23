// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __RECEIVER_H
#define __RECEIVER_H

#include "DelayLockedLoop.h"
#include "ResampleRatioEstimator.h"

#include <readerwriterqueue.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <thread>
#include <string>
#include <memory>

using namespace moodycamel;

class CircularBuffer;
class Filter;

/** A class to manage the reception of audio data. This class executes the 
 *  adaptive resampling algorithm as described by Fons Adriaensen in his
 *  paper "Controlling Adaptive Resampling" from 2012.
 *
 *  http://kokkinizita.linuxaudio.org/papers/adapt-resamp.pdf
 */
class Receiver {
public:
    /** Constructor
     *
     *  \param address the multicast group address.
     *  \param port the UDP port.
     *  \param sampleRate the expected sample rate.
     *  \param periodTime the period time in microseconds.
     *  \param periodSize the period size in frames.
     *  \param channel the number of channels per frame.
     *  \param latency the target latency in number of periods.
     *  \param buffer the circular buffer used to write the audio data to.
     *  \param queue the queue used to retrieve time information from the audio thread from.
     *  \param streaming a flag used to synchronize startup.
     */
    Receiver(const std::string& address, unsigned short port, unsigned int sampleRate,
        unsigned int periodTime, unsigned int periodSize, unsigned int channels, unsigned int latency,
        CircularBuffer& buffer, ReaderWriterQueue<double>& queue, std::atomic<bool>& streaming);

    /** Destructor.
     */
    ~Receiver();

    /** Starts the reception.
     */
    void start();

    /** Stops the reception.
     */
    void stop();

private:
    /** Runs the receive loop.
     */
    void receive();

    const std::string mcastgroup_;          /**< The multicast group address.       */
    const unsigned short port_;             /**< The UDP port.                      */
    const unsigned int periodTime_;         /**< The period time in microseconds.   */
    const unsigned int periodSize_;         /**< The period size in frames.         */
    const unsigned int channels_;           /**< The number of periods per frame.   */
    const unsigned int latency_;            /**< The target latency in periods.     */

    int socket_;                            /**< The UDP socket.                                    */
    struct sockaddr_in addr_;               /**< The socket address information.                    */
    CircularBuffer& buffer_;                /**< The circular buffer used to store the audio data.  */
    std::atomic<bool>& streaming_;          /**< A flag used to synchronize startup.                */
    std::unique_ptr<std::thread> thread_;   /**< The internal network thread.                       */

    unsigned int sampleCount_;              /**< The current count of received samples.             */
    double ratio_;                          /**< The current resampling ratio.                      */
    double tA0, tA1;                        /**< The last and the next timestamps from the audio thread.    */
    unsigned int kA0, kA1;                  /**< The last and the next sample count from the audio thread.  */
    ReaderWriterQueue<double>& timeInfoQueue_;    /**< The time info queue used to retrieve timestamps from the audio thread. */
    DelayLockedLoop dll_;                   /**< The delay-locked loop for the network thread.      */
    ResampleRatioEstimator est_;            /**< The estimator for the resampling ratio.            */
    double err_;                            /**< The current delay error.                           */
};

#endif  // __RECEIVER_H
