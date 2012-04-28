// Copyright 2012 Noriyuki Takahashi.  All Rights Reserved.

// Visit https://code.google.com/apis/console#access to get your own client ID
// and secret.
var CLIENT_ID = '{YOUR_CLIENT_ID}';
var CLIENT_SECRET = '{YOUR_CLIENT_SECRET}';

// Use "installed applications" mechanism for Chrome extension.
var REDIRECT_URI = 'urn:ietf:wg:oauth:2.0:oob';

// Google's oauth2 service.
var GOOGLE_OAUTH2_BASE_URL = 'https://accounts.google.com/o/oauth2';

function jsonToQuery(json) {
  var components = [];
  for (var key in json) {
    components.push(key + '=' + encodeURIComponent(json[key]));
  }
  return components.join('&');
}

function queryToJson(query) {
  var json = {}
  var keyValues = query.split('&');
  for (var i = 0; i < keyValues.length; ++i) {
    var keyValue = keyValues[i].split('=');
    if (keyValue.length != 2) continue;
    json[keyValue[0]] = decodeURIComponent(keyValue[1]);
  }
  return json;
}

// Main procedure to get an access token and its refresh token.
function startGrantProcess() {
  function openTabToLetUserGrantAccess(callback) {
    var query = { response_type: 'code',
                  client_id: CLIENT_ID,
                  redirect_uri: REDIRECT_URI,
                  scope: 'https://www.googleapis.com/auth/userinfo.profile',
                  state: 'NotUsedInThisExample' };
    var url = GOOGLE_OAUTH2_BASE_URL + '/auth?' + jsonToQuery(query);
    chrome.tabs.getCurrent(function(currentTab) {
      var params = { url: url,
                     openerTabId: currentTab.id };
      chrome.tabs.create(params, function(tab) {
        chrome.tabs.onUpdated.addListener(callback);
        function removeListeners(tabId, removeInfo) {
          chrome.tabs.onUpdated.removeListener(callback);
          chrome.tabs.onRemoved.removeListener(removeListeners);
        }
        chrome.tabs.onRemoved.addListener(removeListeners);
      });
    });
  }

  function checkIfTabHasApprovalResponse(tabId, changeInfo, tab, callback) {
    if (changeInfo.status != 'complete') return;
    var urlComponents = tab.url.split('?');
    if (urlComponents.length != 2 ||
        urlComponents[0] != (GOOGLE_OAUTH2_BASE_URL + '/approval')) return;
    var result = tab.title.split(' ');
    if (result.length != 2 ||
        !(result[0] == 'Success' || result[0] == 'Denied')) return;
    chrome.tabs.update(tab.openerTabId, { active: true });
    chrome.tabs.remove(tabId, function() { 
      callback(queryToJson(result[1]));
    });
  }

  function getUserApproval(callback) {
    openTabToLetUserGrantAccess(function (tabId, changeInfo, tab) {
      checkIfTabHasApprovalResponse(tabId, changeInfo, tab, function(res) {
        if (!res) {
          console.error('Response error.');
          return;
        }
        if (res.error) {
          console.info('User denied access request.');
          return;
        }
        callback(res);
      });
    });
  }

  function getRefreshToken(callback) {
    getUserApproval(function(res) {
      var query = { code: res.code,
                    client_id: CLIENT_ID,
                    client_secret: CLIENT_SECRET,
                    redirect_uri: REDIRECT_URI,
                    grant_type: 'authorization_code' };
      var req = new XMLHttpRequest();
      req.open('POST', GOOGLE_OAUTH2_BASE_URL + '/token', true);
      req.onreadystatechange = function() {
        if (req.readyState == 4) {
          var res = JSON.parse(req.responseText);
          if (res.refresh_token) {
            callback(res);
          } else {
            console.error('Failed to get refresh token.');
          }
        }
      };
      req.setRequestHeader('Content-Type', 'application/x-www-form-urlencoded');
      req.send(jsonToQuery(query));
    });
  }

  getRefreshToken(function(res) {
    localStorage['refreshToken'] = res.refresh_token;
    document.getElementById('grant').disabled = true;
    document.getElementById('revoke').disabled = false;
    document.getElementById('token').innerHTML = res.refresh_token;
  });
}

// Main procedure to revoke the current refresh token.
function startRevokeProcess() {
  var refreshToken = localStorage['refreshToken'];
  if (!refreshToken) {
    console.error('refreshToken should be stored when callidng this function');
    return;
  }

  var req = new XMLHttpRequest();
  var query = {
    token: refreshToken
  }
  var url = GOOGLE_OAUTH2_BASE_URL + '/revoke?' + jsonToQuery(query);
  req.open('GET', url, true);
  req.onreadystatechange = function() {
    if (req.readyState != 4) return;
      
    if (req.status == 200) {
      localStorage.removeItem('refreshToken');
      document.getElementById('grant').disabled = false;
      document.getElementById('revoke').disabled = true;
      document.getElementById('token').innerHTML = 'Revoked';
    } else {
      console.error('Failed to revoke the token.');
    }
  };
  req.send();
}

// Initializes the options screen.
window.onload = function() {
  var grantButton = document.getElementById('grant');
  var revokeButton = document.getElementById('revoke');
  var tokenSpan = document.getElementById('token');

  grantButton.onclick = startGrantProcess;
  revokeButton.onclick = startRevokeProcess;

  refreshToken = localStorage['refreshToken'];
  if (refreshToken) {
    grantButton.disabled = true;
    tokenSpan.innerHTML = refreshToken;
  } else {
    revokeButton.disabled = true;
    tokenSpan.value = 'Not yet obtained';
  }
}
