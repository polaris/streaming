// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __PLAYER_H
#define __PLAYER_H

#include "Utils.h"

#include <readerwriterqueue.h>
#include <thread>
#include <memory>
#include <atomic>
#include <alsa/asoundlib.h>
#include <cstddef>

class CircularBuffer;
class Filter;

class Player {
public:
    /** Constructor
     *
     *  \param deviceName the name of the ALSA device.
     *  \param sampleRate the sample rate to be used.
     *  \param periodTime the period time in microseconds.
     *  \param channel the number of channels per frame.
     *  \param latency the target latency in periods.
     *  \param buffer the circular buffer to read the audio data from.
     *  \param timeInfoQueue the queue to transfer the time info to the network thread.
     *  \param streaming a flag to synchronize startup with the network thread.
     */
    Player(const std::string& deviceName, unsigned int sampleRate, unsigned int periodTime,
     unsigned int channels, unsigned int latency, CircularBuffer& buffer, 
     moodycamel::ReaderWriterQueue<double>& timeInfoQueue, std::atomic<bool>& streaming);

    Player(const Player&) = delete;
    Player& operator =(const Player&) = delete;

    /** Destructor.
     */
    ~Player();

    /** Starts the player.
     */
    void start();

    /** Stop the player.
     */
    void stop();

private:
    /** Setup ALSA.
     */
    void setUpAlsa();

    /** Set ALSA hardware parameters.
     */
    void setHardwareParameters();

    /** Set ALSA software parameters.
     */
    void setSoftwareParameters();

    /** Tear down ALSA.
     */
    void tearDownAlsa();

    /** Playback method.
     */
    int playback();

    /** Recovers from buffer over- and under-runs.
     *
     *  \param err the error code.
     */
    int recover(int err);

    const std::string deviceName_;      /**< The name of the ALSA audio device. */
    const unsigned int sampleRate_;     /**< The sample rate.                   */
    const unsigned int periodTime_;     /**< The period time in microseconds.   */
    const unsigned int periodSize_;     /**< The period size in frames.         */
    const unsigned int channels_;       /**< The number of channels per period. */
    const unsigned int latency_;        /**< The target latency in periods.     */

    CircularBuffer& buffer_;                               /**< The circular buffer.   */
    std::atomic<bool>& streaming_;                         /**< The streaming flag.    */
    moodycamel::ReaderWriterQueue<double>& timeInfoQueue_; /**< The queues used to send time info to the network thread. */
    snd_pcm_t* pcm_;                                       /**< ALSA handle.           */
    std::unique_ptr<std::thread> thread_;                  /**< The internal audio thread. */
    std::atomic<bool> running_;                            /**< True if the player is started, otherwise false. */
};

#endif  // __PLAYER_H
