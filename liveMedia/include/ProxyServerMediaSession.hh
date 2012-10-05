/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "liveMedia"
// Copyright (c) 1996-2012 Live Networks, Inc.  All rights reserved.
// A subclass of "ServerMediaSession" that can be used to create a (unicast) RTSP servers that acts as a 'proxy' for
// another (unicast or multicast) RTSP/RTP stream.
// C++ header

#ifndef _PROXY_SERVER_MEDIA_SESSION_HH
#define _PROXY_SERVER_MEDIA_SESSION_HH

#ifndef _SERVER_MEDIA_SESSION_HH
#include "ServerMediaSession.hh"
#endif
#ifndef _MEDIA_SESSION_HH
#include "MediaSession.hh"
#endif

// A subclass of "RTSPClient", used to refer to the particular "ProxyServerMediaSession" object being used.
// It is used only within the implementation of "ProxyServerMediaSession", but is defined here, in case developers wish to
// subclass it.

class ProxyRTSPClient: public RTSPClient {
public:
  ProxyRTSPClient(class ProxyServerMediaSession& ourServerMediaSession, char const* rtspURL,
                  char const* username, char const* password,
                  portNumBits tunnelOverHTTPPortNum, int verbosityLevel);
  virtual ~ProxyRTSPClient();

  void continueAfterDESCRIBE(char const* sdpDescription);
  void continueAfterOPTIONS(int resultCode);
  void continueAfterSETUP();

private:
  void reset();

  Authenticator* auth() { return fOurAuthenticator; }

  void scheduleLivenessCommand();
  static void sendLivenessCommand(void* clientData);

  void scheduleDESCRIBECommand();
  static void sendDESCRIBE(void* clientData);

  static void subsessionTimeout(void* clientData);
  void handleSubsessionTimeout();

private:
  friend class ProxyServerMediaSession;
  friend class ProxyServerMediaSubsession;
  ProxyServerMediaSession& fOurServerMediaSession;
  char* fOurURL;
  Authenticator* fOurAuthenticator;
  Boolean fStreamRTPOverTCP;
  class ProxyServerMediaSubsession *fSetupQueueHead, *fSetupQueueTail;
  unsigned fNumSetupsDone;
  TaskToken fLivenessCommandTask, fDESCRIBECommandTask, fSubsessionTimerTask;
  unsigned fNextDESCRIBEDelay; // in seconds
  Boolean fLastCommandWasPLAY;
};


class ProxyServerMediaSession: public ServerMediaSession {
public:
  static ProxyServerMediaSession* createNew(UsageEnvironment& env,
					    char const* inputStreamURL, // the "rtsp://" URL of the stream we'll be proxying
					    char const* streamName = NULL,
					    char const* username = NULL, char const* password = NULL,
					    portNumBits tunnelOverHTTPPortNum = 0,
					        // for streaming the *proxied* (i.e., back-end) stream
					    int verbosityLevel = 0);
      // Hack: "tunnelOverHTTPPortNum" == 0xFFFF (i.e., all-ones) means: Stream RTP/RTCP-over-TCP, but *not* using HTTP
      // "verbosityLevel" == 1 means display basic proxy setup info; "verbosityLevel" == 2 means display RTSP client protocol also.
  virtual ~ProxyServerMediaSession();

  char const* url() const;

  char describeCompletedFlag;
    // initialized to 0; set to 1 when the back-end "DESCRIBE" completes.
    // (This can be used as a 'watch variable' in "doEventLoop()".)
  Boolean describeCompletedSuccessfully() const { return fClientMediaSession != NULL; }
    // This can be used - along with "describeCompletdFlag" - to check whether the back-end "DESCRIBE" completed *successfully*.

protected:
  ProxyServerMediaSession(UsageEnvironment& env, char const* inputStreamURL, char const* streamName,
			  char const* username, char const* password, portNumBits tunnelOverHTTPPortNum, int verbosityLevel);

  // If you subclass "ProxyRTSPClient", then you should also subclass "ProxyServerMediaSession" and redefine this virtual function
  // in order to create new objects of your "ProxyRTSPClient" subclass:
  virtual ProxyRTSPClient* createNewProxyRTSPClient(char const* rtspURL, char const* username, char const* password,
						    portNumBits tunnelOverHTTPPortNum, int verbosityLevel);

protected:
  ProxyRTSPClient* fProxyRTSPClient;
  MediaSession* fClientMediaSession;

private:
  friend class ProxyRTSPClient;
  friend class ProxyServerMediaSubsession;
  void continueAfterDESCRIBE(char const* sdpDescription);

private:
  int fVerbosityLevel;
  class PresentationTimeSessionNormalizer* fPresentationTimeSessionNormalizer;
};

#endif
