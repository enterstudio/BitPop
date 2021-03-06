var friendList = null;
var myUid = null;
var statuses = {};
var inboxData = null;
var inboxFetchInterval = null;
var newMessageAudio = new Audio("mouth_pop.wav");
var loggedIn = false;

//chrome.extension.sendRequest(bitpop.CONTROLLER_EXTENSION_ID,
//  { type: 'observe',
//    extensionId: chrome.i18n.getMessage('@@extension_id')
//  });

// Close Google Docs extension options window, appearing on first run
// and focus the Sign In page.
if (!localStorage.firstRunCompleted) {
  chrome.tabs.onUpdated.addListener(function(tabId, changeInfo, tab) {
    if (changeInfo && changeInfo.url &&
        changeInfo.url.indexOf('chrome-extension://nnbmlagghjjcbdhgmkedmbmedengocbn/options.html') == 0) {
      chrome.tabs.remove(tabId);
      chrome.tabs.query({ url: "chrome://signin/*" }, function (tabList) {
        if (tabList.length !== 1)
          return;
        chrome.tabs.update(tabList[0].id, { active: true });
      });
      localStorage.setItem("firstRunCompleted", true);
      chrome.tabs.onUpdated.removeListener(arguments.callee);
    }
  });
}

(function () {
  if (chrome.browserAction)
    chrome.browserAction.onClicked.addListener(function (tab) {
      chrome.bitpop.facebookChat.getFriendsSidebarVisible(function(is_visible) {
        chrome.bitpop.facebookChat.setFriendsSidebarVisible(!is_visible);
        //onSuppressChatChanged();
      });
    });
  else
    setTimeout(arguments.callee, 1000);
})();

// force myUid fetch if it wasn't retrieved successfully
// or after bg page crash
setTimeout(
  function() {
    if (!myUid) {
      chrome.extension.sendMessage(
        bitpop.CONTROLLER_EXTENSION_ID,
        { type: 'getMyUid' },
        function(response) {
          if (response && response.id) {
            myUid = response.id;
            chrome.extension.sendMessage(
              bitpop.CONTROLLER_EXTENSION_ID,
              { type: 'forceFriendListSend' }
            );
          }
        }
      );
    }
  },
  5000);

chrome.extension.onMessage.addListener(function (request, sender, sendResponse) {
  if (!request.type)
    return false;

  if (request.type == 'setStatusMessage') {
    chrome.extension.sendMessage(bitpop.CONTROLLER_EXTENSION_ID,
      { type: 'setFacebookStatusMessage',
        msg: request.msg
      },
      function (response) {
        sendResponse(response);
      }
    );
    return true;
  }
});

chrome.extension.onMessageExternal.addListener(function (request, sender, sendResponse) {
  if (!request.type)
    return;

  if (request.type == 'myUidAvailable') {
      myUid = request.myUid;
      sendInboxRequest();
      // request inbox again in 30 minutes
      if (inboxFetchInterval) { clearInterval(inboxFetchInterval); }
      inboxFetchInterval = setInterval(function() {
        if (myUid && friendList) { sendInboxRequest(); }
       }, 1000 * 60 * 30);
      loggedIn = true;
      onSuppressChatChanged();
  } else if (request.type == 'friendListReceived') {
    if (!friendList) {
      // send status notifications so that every visible chat button
      // has correct statuses
      for (var i = 0; i < request.data.length; ++i) {
        chrome.bitpop.facebookChat.newIncomingMessage(request.data[i].uid.toString(), "",
          request.data[i].online_presence || 'offline', "");
      }
    }

    friendList = request.data;

    sendStatusesRequest();

  } else if (request.type == 'loggedOut') {
    if (inboxFetchInterval) { clearInterval(inboxFetchInterval); inboxFetchInterval = null; }
    statuses = {};
    friendList = null;
    loggedIn = false;
    onSupressChatChanged();
  } else if (request.type == 'wentOffline') {
    if (inboxFetchInterval) { clearInterval(inboxFetchInterval); inboxFetchInterval = null; }
    statuses = {};
    friendList = null;
  } else if (request.type == 'chatIsIdle') {
    statuses = {};
  } else if (request.type == 'userStatusChanged') {
    // send change status message: empty message body signals to only check
    // for status change
    chrome.bitpop.facebookChat.newIncomingMessage(request.uid.toString(), "",
        request.status, "");

    // set global variable storing each user status, reported by XMPP
    statuses[request.uid.toString()] = request.status;

  } else if (request.type == 'newMessage') {
    var msgDate = new Date();  // set 'now' as the message arrive time

    console.assert(myUid != null);

    bitpop.saveToLocalStorage(myUid, request.from,
      bitpop.preprocessMessageText(request.body),
      msgDate,
      false);

    var found = false;
    var vs = chrome.extension.getViews();
    for (var i = 0; i < vs.length; ++i) {
      if (vs[i].location.hash.length > 1 && vs[i].location.hash.slice(1).split('&')[0] == request.from) {
        found = true;
        break;
      }
    }

    if (!found) {
      for (var i = 0; i < friendList.length; ++i) {
        if (friendList[i].uid == request.from) {
          // use status from fql result first,
          // then from xmpp server status update,
          // else set it to offline
          var status = null;
          if (friendList[i].online_presence !== null)
            status = friendList[i].online_presence;
          else if (statuses[friendList[i].uid.toString()])
            status = statuses[friendList[i].uid.toString()];
          else
            status = 'offline';

          chrome.bitpop.facebookChat.newIncomingMessage(
                             friendList[i].uid.toString(),
                             friendList[i].name,
                             status,
                             request.body);
          break;
        }
      }
    }

    newMessageAudio.play();
  } else if (request.type == 'typingStateChanged') {
    if (request.isTyping) {
      chrome.bitpop.facebookChat.newIncomingMessage(request.uid.toString(), "",
          'composing', "");
    } else {
      chrome.bitpop.facebookChat.newIncomingMessage(request.uid.toString(), "",
          'active', "");
    }

    if (friendList) {
      for (var i = 0; i < friendList.length; ++i) {
        if (friendList[i].uid == request.uid) {
          friendList[i].isTyping = request.isTyping;
          break;
        }
      }
    }
  }
});

function sendInboxRequest() {
  chrome.extension.sendMessage(bitpop.CONTROLLER_EXTENSION_ID,
    { type: 'graphApiCall',
      path: '/me/inbox',
      params: {}
    },
    function (response) {
      inboxData = response.data;
      replaceLocalHistory(inboxData);
      chrome.extension.sendMessage({ type: 'inboxDataAvailable' });
    }
  );
}

function sendStatusesRequest() {
  chrome.extension.sendMessage(bitpop.CONTROLLER_EXTENSION_ID,
    { type: 'graphApiCall',
      path: '/me/statuses',
      params: { 'limit': '1' }
    },
    function (response) {
      if (!response.data || !response.data.length || !response.data[0].message)
        return;
      chrome.extension.sendMessage({ type: 'statusMessageUpdate',
                                     msg: response.data[0].message });
    }
  );
}

function replaceLocalHistory(data) {
  console.assert(myUid != null);
  console.assert(data != null);

  for (var i = 0; i < data.length; i++) {
    var to_ids = [];
    for (var j = 0; j < data[i].to.data.length; j++) {
      to_ids.push(data[i].to.data[j].id);
      if (to_ids[j] == myUid) // exclude my uid from to_ids list
        to_ids.pop();
    }

    if (to_ids.length > 1 || to_ids.length == 0)
      continue;

    var jid = myUid + ':' + to_ids[0].toString();

    localStorage[jid + '.thread_id'] = data[i].id;

    if (!data[i].comments || !data[i].comments.data)
      continue;

    localStorage.removeItem(jid);

    for (var j = 0; j < data[i].comments.data.length; j++) {
      bitpop.saveToLocalStorage(myUid, to_ids[0],
          bitpop.preprocessMessageText(data[i].comments.data[j].message),
          (new Date(data[i].comments.data[j].created_time)).getTime(),
          data[i].comments.data[j].from.id == myUid
      );
    }
  }
}

// ********************************************************
// facebook chat enable/disable functionality
//
// ********************************************************
//
// the list of tab ids which this extension is interested in.
// Mostly facebook.com tabs
var fbTabs = {};

var TRACE = 0;
var DEBUG = 1;
var INFO  = 2;
var logLevel = DEBUG;

function myLog( ) {
    if(logLevel <= DEBUG) {
        console.log(arguments);
    }
}

chrome.tabs.onRemoved.addListener(
    function(tab)
    {
        delete fbTabs[tab];
    }
);

function sendResponseToContentScript(sender, data, status, response)
{
    if(chrome.extension.lastError) {
        status = "error";
        response = chrome.extension.lastError;
    }
    myLog("Sending response ", data.action, data.id, status, response);
    sender({
        action: data.action,
        id: data.id,
        status: status,
        response: response
    });
}

/**
 * extract the doman name from a URL
 */
function getDomain(url) {
   return url.match(/:\/\/(.[^/]+)/)[1];
}

function addFbFunctionality( )
{
    // add a listener to events coming from contentscript
    chrome.extension.onMessage.addListener(
        function(request, sender, sendResponse) {
            if (typeof request != 'string')
              return false;
            myLog("Received request ", request);
            if(request) {
                var data = JSON.parse(request);
                try {
                    if(data.action === 'chat' && data.friendId !== undefined) {
                        // chat request event
                        //fbfeed.openChatWindow(data.friendId, function(a) {
                        //    sendResponseToContentScript(sendResponse, data, "ok", a);
                        //});
                    } else if(data.action === 'shouldEnableFbJewelsAndChat' &&
                        data.userId !== undefined) {
                        // save the id of the tabs which want the Jewel/Chat enable/disable
                        // so that they can be informed when quite mode changes
                        fbTabs[sender.tab.id] = {value: 1, injected: true};
                        if(sender.tab.incognito) {
                            // if the broser is in incognito mode make a local decision
                            // no need to consult the native side.
                            var response =  {
                                    enableChat: true,
                                    enableJewels: true
                                };
                            sendResponseToContentScript(sendResponse, data, "ok", response);
                        } else {
                            //chrome.bitpop.facebookChat.getFriendsSidebarVisible(function(is_visible) {
                            chrome.bitpop.prefs.facebookShowChat.get({}, function(details) {
                              var facebookShowChat = details.value;
                              //chrome.bitpop.prefs.facebookShowJewels.get({}, function(details2) {
                              //  var facebookShowJewels = details2.value;
                              var response = null;
                              if (loggedIn) {
                                response = {
                                  enableChat:   facebookShowChat,
                                  enableJewels: true
                                };
                              }
                              else {
                                response = { enableChat:true, enableJewels:true };
                              }

                              sendResponseToContentScript(sendResponse, data,
                                                          "ok", response);
                              //});
                            });
                            //});
                        }
                    }
                } catch(e) {
                    sendResponseToContentScript(sendResponse, data, "error", e);
                }
            }
            return true;
        });
}

function onSuppressChatChanged(details) {
  for(var i in fbTabs) {
    if(fbTabs[i].injected) {
        var id = parseInt(i);
        chrome.tabs.sendMessage(
            id,
            {},
            function(responseData) {
            // ignore nothing to do here.
            });
    }
  }
}

chrome.bitpop.prefs.facebookShowChat.onChange.addListener(onSuppressChatChanged);
chrome.bitpop.prefs.facebookShowJewels.onChange.addListener(onSuppressChatChanged);

addFbFunctionality();
