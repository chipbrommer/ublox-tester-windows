#pragma once
/////////////////////////////////////////////////////////////////////////////
//
//  Applied Systems Engineering Inc. proprietary rights are included in the
//  information disclosed herein.
//
//  Recipient by accepting this document or software agrees that neither
//  this software nor the information disclosed herein nor any part thereof
//  shall be reproduced or transferred to other documents or software or used
//  or disclosed to others for any purpose except as specifically authorized
//  in writing by:
//
//                     Applied Systems Engineering Inc
//                            Niceville, Florida
//
/////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
//	Programmer:		C. M. Ditto
//
//	Description:    Declares the minimum interface that all GPS receiver's
//                  must support
//
//	Date:			Jan. 29, 2021
//
//  Include files:
//          name                        reason included
//          --------------------        ---------------------------------------
#include    <memory>                //  pointer types
//
///////////////////////////////////////////////////////////////////////////////

struct IGPSReceiver
{
    using Pointer = std::shared_ptr<IGPSReceiver>;

    virtual void startInterface() = 0;
    virtual void do_100_HzProcessing() = 0;
    virtual void getPosition(double& latRadians, double& lonRadians, double& haeMeters) = 0;
    virtual void getUncertainties(double& latStdDevMeters, double& lonStdDevMeters, double& altStdDevMeters) = 0;
    virtual double getUndulationMeters() = 0;
    virtual void getVelocity(double& northMetersPerSec, double& eastMetersPerSec, double& downMetesPerSec) = 0;
    virtual void getGPSTimeOfDataValidity(unsigned short& week, double& secondsOfWeek) = 0;
    virtual unsigned int getLeapSeconds() = 0;
    virtual bool isSolutionValid() = 0;
    virtual bool positionUpdateOccured() = 0;
    virtual void clearPositionUpdateOccured() = 0;
    virtual bool velocityUpdateOccured() = 0;
    virtual void clearVelocityUpdateOccured() = 0;
    virtual void getFOMs(unsigned short& fom, unsigned short& tfom) = 0;
    virtual bool newPVTAvailable() = 0;
    virtual void clearNewPVTAvailable() = 0;
};