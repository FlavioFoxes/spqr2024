/**
 * @file LEDHandler.cpp
 * This file implements a module that generates the LEDRequest from certain representations.
 * @author jeff
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "LEDHandler.h"

#include <algorithm>

void LEDHandler::update(LEDRequest& ledRequest)
{
  //reset
  FOREACH_ENUM(LEDRequest::LED, led)
    ledRequest.ledStates[led] = LEDRequest::off;

  setRightEye(ledRequest);
  setLeftEye(ledRequest);
  setChestButton(ledRequest);
  setLeftFoot(ledRequest);
  setRightFoot(ledRequest);

  setWhistleLeds(ledRequest);

  //update
  setRightEar(ledRequest);
  setLeftEar(ledRequest);
  setHead(ledRequest);
}

void LEDHandler::setRightEar(LEDRequest& ledRequest)
{
  //right ear -> battery
  setBatteryLevelInEar(ledRequest, LEDRequest::earsRight0Deg);
}

void LEDHandler::setLeftEar(LEDRequest& ledRequest)
{
  //left ear -> connected players
  //          + GameController connection lost -> freaky blinking
  if(theFrameInfo.getTimeSince(theGameInfo.timeLastPacketReceived) > gameControllerTimeOut)
  {
    ledRequest.ledStates[LEDRequest::earsLeft324Deg] = LEDRequest::blinking;
    ledRequest.ledStates[LEDRequest::earsLeft144Deg] = LEDRequest::blinking;
  }

  int numberOfConnectedTeammates = static_cast<int>(theTeamData.teammates.size());
  if(numberOfConnectedTeammates > 0)
  {
    ledRequest.ledStates[LEDRequest::earsLeft0Deg] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::earsLeft36Deg] = LEDRequest::on;
  }
  if(numberOfConnectedTeammates > 1)
  {
    ledRequest.ledStates[LEDRequest::earsLeft72Deg] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::earsLeft108Deg] = LEDRequest::on;
  }
  if(numberOfConnectedTeammates > 2)
  {
    ledRequest.ledStates[LEDRequest::earsLeft180Deg] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::earsLeft216Deg] = LEDRequest::on;
  }
  if(numberOfConnectedTeammates > 3)
  {
    ledRequest.ledStates[LEDRequest::earsLeft252Deg] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::earsLeft288Deg] = LEDRequest::on;
  }
}

void LEDHandler::setEyeColor(LEDRequest& ledRequest,
                             bool left,
                             EyeColor col,
                             LEDRequest::LEDState s)
{
  LEDRequest::LED first = left ? LEDRequest::faceLeftRed0Deg : LEDRequest::faceRightRed0Deg;

  static const int redOffset = 0,
                   greenOffset = LEDRequest::faceLeftGreen0Deg - LEDRequest::faceLeftRed0Deg,
                   blueOffset = LEDRequest::faceLeftBlue0Deg - LEDRequest::faceLeftRed0Deg,
                   numOfLEDsPerColor = LEDRequest::faceLeftRed315Deg - LEDRequest::faceLeftRed0Deg;

  LEDRequest::LEDState halfState = s == LEDRequest::off ? LEDRequest::off : LEDRequest::half;

  switch(col)
  {
    case red:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + redOffset + i] = s;
      break;
    case green:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + greenOffset + i] = s;
      break;
    case blue:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + blueOffset + i] = s;
      break;
    case white:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + redOffset + i] = s;
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + greenOffset + i] = s;
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + blueOffset + i] = s;
      break;
    case magenta:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + redOffset + i] = halfState;
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + blueOffset + i] = s;
      break;
    case yellow:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + greenOffset + i] = halfState;
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + redOffset + i] = s;
      break;
    case cyan:
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + greenOffset + i] = halfState;
      for(int i = 0; i <= numOfLEDsPerColor; i++)
        ledRequest.ledStates[first + blueOffset + i] = s;
      break;
    default:
      FAIL("Unknown color.");
      break;
  }
}

void LEDHandler::setLeftEye(LEDRequest& ledRequest)
{
  //no groundContact
  if(!theGroundContactState.contact/* && (theFrameInfo.time & 512)*/)
    setEyeColor(ledRequest, true, yellow, LEDRequest::on);
  else
  {
    if (theGameInfo.state == STATE_STANDBY){
      if (theRefereeEstimator.measures > 0)
        setEyeColor(ledRequest, true, green, LEDRequest::on);
      else
        setEyeColor(ledRequest, true, red, LEDRequest::on);
      return;
    }
    bool ballSeen = theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen) < 250;
    bool featureSeen = theFrameInfo.getTimeSince(theFieldFeatureOverview.combinedStatus.lastSeen) < 250;

    if(ballSeen && featureSeen)
      setEyeColor(ledRequest, true, red, LEDRequest::on);
    else if(ballSeen)
      setEyeColor(ledRequest, true, white, LEDRequest::on);
    else if(featureSeen)
      setEyeColor(ledRequest, true, blue, LEDRequest::on);
  }
}

void LEDHandler::setWhistleLeds(LEDRequest& ledRequest){
  if (theFrameInfo.getTimeSince(theWhistle.lastTimeWhistleDetected) < 200){
    for (int i = 0; i < 10; i++)
    {
      ledRequest.ledStates[LEDRequest::earsRight0Deg + i] = LEDRequest::fastBlinking;
      ledRequest.ledStates[LEDRequest::earsLeft0Deg + i] = LEDRequest::fastBlinking;
    }
    for (unsigned i = LEDRequest::headRearLeft0; i <= LEDRequest::headMiddleLeft0; i++)
      ledRequest.ledStates[i] = LEDRequest::fastBlinking;
    ledRequest.ledStates[LEDRequest::chestRed] = LEDRequest::fastBlinking;
    ledRequest.ledStates[LEDRequest::chestGreen] = LEDRequest::fastBlinking;
  }
}

void LEDHandler::setRightEye(LEDRequest& ledRequest)
{
  if (theGameInfo.state == STATE_STANDBY){
    if (theRefereeEstimator.measures > 0)
      setEyeColor(ledRequest, true, green, LEDRequest::on);
    else
      setEyeColor(ledRequest, true, red, LEDRequest::on);
    return;
  }
  // SPQR 2022: right eye color is role
  const int numOfLEDsPerColor = LEDRequest::faceLeftRed315Deg - LEDRequest::faceLeftRed0Deg;
  if(thePlayerRole.role == PlayerRole::striker){
    setEyeColor(ledRequest, false, red, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::libero){
    setEyeColor(ledRequest, false, white, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::supporter){
    setEyeColor(ledRequest, false, green, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::jolly){
    setEyeColor(ledRequest, false, yellow, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::defenderone){
    setEyeColor(ledRequest, false, blue, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::defendertwo){
    setEyeColor(ledRequest, false, cyan, LEDRequest::on);
  }
  else if(thePlayerRole.role == PlayerRole::passiveSearcher || thePlayerRole.role == PlayerRole::activeSearcher){
    setEyeColor(ledRequest, false, magenta, LEDRequest::on);
  }
  else{
    setEyeColor(ledRequest, false, white, LEDRequest::off);
  }
}

void LEDHandler::setHead(LEDRequest& ledRequest)
{
  for(unsigned i = LEDRequest::headRearLeft0; i <= LEDRequest::headMiddleLeft0; i++)
    ledRequest.ledStates[i] = LEDRequest::off;

  if(theSystemSensorData.batteryCharging)
  {
    for(LEDRequest::LED i = LEDRequest::firstHeadLED; i <= LEDRequest::lastHeadLED; i = LEDRequest::LED(unsigned(i) + 1))
      ledRequest.ledStates[i] = LEDRequest::off;

    ++chargingLED %= (LEDRequest::numOfHeadLEDs * chargingLightSlowness);
    const LEDRequest::LED currentLED = headLEDCircle[chargingLED / chargingLightSlowness];
    const LEDRequest::LED nextLED = headLEDCircle[(chargingLED / chargingLightSlowness + 1u) % LEDRequest::numOfHeadLEDs];
    ledRequest.ledStates[currentLED] = LEDRequest::on;
    ledRequest.ledStates[nextLED] = LEDRequest::on;
  }

  // TODO restore MessageManagement

  else if (theMessageManagement.outOfPackets && theGameInfo.state == STATE_PLAYING) {
    for(LEDRequest::LED i = LEDRequest::firstHeadLED; i <= LEDRequest::lastHeadLED; i = LEDRequest::LED(unsigned(i) + 1))
      ledRequest.ledStates[i] = LEDRequest::off;

    const int FRAMES = 3;
    int anim_frame = (theFrameInfo.time / outOfPacketsLightSlowness) % FRAMES;
    switch (anim_frame) {
      case 0:
        ledRequest.ledStates[LEDRequest::headFrontLeft1] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headFrontRight1] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearLeft2] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearRight2] = LEDRequest::on;
        break;
      case 1:
        ledRequest.ledStates[LEDRequest::headFrontLeft0] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headFrontRight0] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearLeft1] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearRight1] = LEDRequest::on;
        break;
      case 2:
        ledRequest.ledStates[LEDRequest::headMiddleLeft0] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headMiddleRight0] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearLeft0] = LEDRequest::on;
        ledRequest.ledStates[LEDRequest::headRearRight0] = LEDRequest::on;
        break;
    }
  }
}

void LEDHandler::setChestButton(LEDRequest& ledRequest)
{
  switch(theRobotInfo.mode)
  {
    case RobotInfo::unstiff:
      ledRequest.ledStates[LEDRequest::chestBlue] = LEDRequest::blinking;
      break;
    case RobotInfo::calibration:
      ledRequest.ledStates[LEDRequest::chestRed] = LEDRequest::on;
      ledRequest.ledStates[LEDRequest::chestBlue] = LEDRequest::on;
      break;
    case RobotInfo::active:
    default:
      if(theRobotInfo.penalty != PENALTY_NONE)
        ledRequest.ledStates[LEDRequest::chestRed] = LEDRequest::on;
      else
        switch(theGameInfo.state)
        {
          case STATE_STANDBY:
            ledRequest.ledStates[LEDRequest::chestGreen] = LEDRequest::on;
            ledRequest.ledStates[LEDRequest::chestBlue] = LEDRequest::on;
            break;
          case STATE_READY:
            ledRequest.ledStates[LEDRequest::chestBlue] = LEDRequest::on;
            break;
          case STATE_SET:
            ledRequest.ledStates[LEDRequest::chestRed] = LEDRequest::on;
            ledRequest.ledStates[LEDRequest::chestGreen] = LEDRequest::half;
            break;
          case STATE_PLAYING:
            ledRequest.ledStates[LEDRequest::chestGreen] = LEDRequest::on;
            break;
        }
  }
}

void LEDHandler::setLeftFoot(LEDRequest& ledRequest)
{
  switch(theOwnTeamInfo.fieldPlayerColour)
  {
    case TEAM_ORANGE:
      ledRequest.ledStates[LEDRequest::footLeftGreen] = LEDRequest::half;
    case TEAM_RED:
      ledRequest.ledStates[LEDRequest::footLeftRed] = LEDRequest::on;
      break;
    case TEAM_WHITE:
      ledRequest.ledStates[LEDRequest::footLeftBlue] = LEDRequest::on;
    case TEAM_YELLOW:
      ledRequest.ledStates[LEDRequest::footLeftRed] = LEDRequest::on;
    case TEAM_GREEN:
      ledRequest.ledStates[LEDRequest::footLeftGreen] = LEDRequest::on;
      break;
    case TEAM_PURPLE:
      ledRequest.ledStates[LEDRequest::footLeftRed] = LEDRequest::on;
    case TEAM_BLUE:
      ledRequest.ledStates[LEDRequest::footLeftBlue] = LEDRequest::on;
      break;
    case TEAM_GRAY:
      ledRequest.ledStates[LEDRequest::footLeftBlue] = LEDRequest::half;
    case TEAM_BROWN: // more a darker yellow
      ledRequest.ledStates[LEDRequest::footLeftRed] = LEDRequest::half;
      ledRequest.ledStates[LEDRequest::footLeftGreen] = LEDRequest::half;
      break;
  }
}

void LEDHandler::setRightFoot(LEDRequest& ledRequest)
{
  if(theGameInfo.state == STATE_INITIAL &&
     theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT &&
     theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber)
    ledRequest.ledStates[LEDRequest::footRightGreen] = LEDRequest::on;
  else if(theGameInfo.state == STATE_INITIAL &&
          theGameInfo.gamePhase == GAME_PHASE_PENALTYSHOOT &&
          theGameInfo.kickingTeam != theOwnTeamInfo.teamNumber)
  {
    ledRequest.ledStates[LEDRequest::footRightRed] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::footRightGreen] = LEDRequest::on;
  }
  else if(theFrameInfo.getTimeSince(theGameInfo.timeLastPacketReceived) < gameControllerTimeOut &&
          theGameInfo.state <= STATE_SET &&
          theGameInfo.kickingTeam == theOwnTeamInfo.teamNumber)
  {
    ledRequest.ledStates[LEDRequest::footRightRed] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::footRightGreen] = LEDRequest::on;
    ledRequest.ledStates[LEDRequest::footRightBlue] = LEDRequest::on;
  }
}

void LEDHandler::setBatteryLevelInEar(LEDRequest& ledRequest, LEDRequest::LED baseLED)
{
  int onLEDs = std::min(static_cast<int>(theSystemSensorData.batteryLevel / 0.1f), 9);

  for(int i = 0; i <= onLEDs; ++i)
    ledRequest.ledStates[baseLED + i] = LEDRequest::on;
}

MAKE_MODULE(LEDHandler, behaviorControl);
