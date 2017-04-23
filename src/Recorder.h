// © 2017 Jan Deinhard.
// Distributed under the BSD license.

#ifndef __RECORDER_H
#define __RECORDER_H

#include <thread>
#include <memory>
#include <atomic>
#include <alsa/asoundlib.h>
#include <cstddef>

class Transmitter;
class PacketPool;

/** A class used to capture real-time audio data from ALSA.
 */
class Recorder {
public:
    /** Modes.
     */
    enum class Mode {
        Capture,
        Click
    };

    /** Constructor.
     *
     *  \param deviceName the name of the ALSA device.
     *  \param sampleRate the sample rate.
     *  \param periodTime the period time in microseconds.
     *  \param channel the number of channels.
     *  \param mode the mode (either Capture or Click).
     *  \param transmitter a reference to the transmitter.
     *  \param poll a pool of packets.
     */
    Recorder(const std::string& deviceName, unsigned int sampleRate, unsigned int periodTime, 
        unsigned int channels, Mode mode, Transmitter& transmitter, PacketPool& pool);

    Recorder(const Recorder&) = delete;
    Recorder& operator =(const Recorder&) = delete;

    /** Destructor.
     */
    ~Recorder();

    /** Starts the recorder.
     */
    void start();

    /** Stops the recorder.
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

    /** Capture method.
     */
    int capture();

    /** Recovers from buffer over- and under-runs.
     *
     *  \param err the error code.
     */
    int recover(int err);

    const std::string deviceName_;      /**< The name of the ALSA audio device.     */
    const unsigned int sampleRate_;     /**< The sample rate.                       */
    const unsigned int periodTime_;     /**< The period time in microseconds.       */
    const unsigned int periodSize_;     /**< The period size in frames.             */
    const unsigned int channels_;       /**< The number of channels per period.     */
    const Mode mode_;                   /**< The mode used to generate audio data.  */

    Transmitter& transmitter_;          /**< The transmitter used to send the packets.       */
    PacketPool& pool_;                  /**< A pool of packets.                              */
    snd_pcm_t* pcm_;                    /**< ALSA handle.                                    */
    std::unique_ptr<std::thread> thread_;   /**< The internal audio thread.                  */
    std::atomic<bool> running_;         /**< True if the player is started, otherwise false. */
};

#endif  // __RECORDER_H
