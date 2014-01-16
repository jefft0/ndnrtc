//
//  video-receiver.cpp
//  ndnrtc
//
//  Copyright 2013 Regents of the University of California
//  For licensing details see the LICENSE file.
//
//  Author:  Peter Gusev
//

//#undef NDN_TRACE

#include "video-receiver.h"
#include "ndnrtc-utils.h"

using namespace std;
using namespace webrtc;
using namespace ndnrtc;

//******************************************************************************
//******************************************************************************
#pragma mark - construction/destruction
NdnVideoReceiver::NdnVideoReceiver(const ParamsStruct &params) :
NdnMediaReceiver(params),
frameConsumer_(nullptr)
{
    fetchAhead_ = 9; // fetch segments ahead
    playoutBuffer_ = new VideoPlayoutBuffer();
}

NdnVideoReceiver::~NdnVideoReceiver()
{
}

//******************************************************************************
#pragma mark - public
int NdnVideoReceiver::startFetching()
{
    int res = NdnMediaReceiver::startFetching();
    
    if (RESULT_GOOD(res))
        INFO("video fetching started");
    
    return res;
}

//******************************************************************************
#pragma mark - private
void NdnVideoReceiver::playbackPacket()
{
    jitterTiming_.startFramePlayout();
    
    int frameno = -1;
    FrameBuffer::Slot* slot;
    shared_ptr<EncodedImage> frame = ((VideoPlayoutBuffer*)playoutBuffer_)->acquireNextFrame(&slot);
    
    // render frame if we have one
    if (frame.get() && frameConsumer_)
    {
        
        frameno = slot->getFrameNumber();
        nPlayedOut_++;
        frameLogger_->log(NdnLoggerLevelInfo,
                          "PLAYOUT: \t%d \t%d \t%d \t%d \t%d \t%ld \t%ld \t%.2f",
                          slot->getFrameNumber(),
                          slot->assembledSegmentsNumber(),
                          slot->totalSegmentsNumber(),
                          slot->isKeyFrame(),
                          playoutBuffer_->getJitterSize(),
                          slot->getAssemblingTimeUsec(),
                          frame->capture_time_ms_,
                          currentProducerRate_);
        
        frameConsumer_->onEncodedFrameDelivered(*frame.get());
    }
    
    // get playout time (delay) for the rendered frame
    int framePlayoutTime = ((VideoPlayoutBuffer*)playoutBuffer_)->releaseAcquiredSlot();
    jitterTiming_.updatePlayoutTime(framePlayoutTime);
    
    // setup and run playout timer for calculated playout interval
    jitterTiming_.runPlayoutTimer();
}

void NdnVideoReceiver::switchToMode(NdnVideoReceiver::ReceiverMode mode)
{
    NdnMediaReceiver::switchToMode(mode);
    
    // empty
}

bool NdnVideoReceiver::isLate(unsigned int frameNo)
{
    return (frameNo < playoutBuffer_->getPlayheadPointer());
}
