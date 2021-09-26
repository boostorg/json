//
// Copyright (c) 2021 Dmitry Arkhipov (grisumbras@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/json
//

#include <boost/json/iostreams.hpp>

#include <sstream>
#include <fstream>

#include "test_suite.hpp"

BOOST_JSON_NS_BEGIN

namespace {

constexpr auto& json_text = R"EOF(
[
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:30Z",
    "actor": {
      "gravatar_id": "a7cec1f75a06a5f8ab53139515da5d99",
      "login": "jathanism",
      "avatar_url": "https://secure.gravatar.com/avatar/a7cec1f75a06a5f8ab53139515da5d99?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/jathanism",
      "id": 138052
    },
    "repo": {
      "url": "https://api.github.com/repos/jathanism/trigger",
      "id": 6357414,
      "name": "jathanism/trigger"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/jathanism/trigger/commits/05570a3080693f6e55244e012b3b1ec59516c01b",
          "message": "- SSH Channel data now initialized in base class (TriggerSSHChannelBase)\n- New doc w/ checklist for adding new vendor support to Trigger.",
          "distinct": true,
          "sha": "05570a3080693f6e55244e012b3b1ec59516c01b",
          "author": {
            "email": "jathanism@aol.com",
            "name": "jathanism"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/issue-22",
      "push_id": 134107894,
      "head": "05570a3080693f6e55244e012b3b1ec59516c01b",
      "before": "7460e1588817b3f885fb4ec76ec2f08c7caf6385",
      "size": 1
    },
    "id": "1652857722"
  },
  {
    "type": "CreateEvent",
    "created_at": "2013-01-10T07:58:29Z",
    "actor": {
      "gravatar_id": "51c8c8adbe8abff73c622a734afae4b0",
      "login": "noahlu",
      "avatar_url": "https://secure.gravatar.com/avatar/51c8c8adbe8abff73c622a734afae4b0?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/noahlu",
      "id": 1229684
    },
    "repo": {
      "url": "https://api.github.com/repos/noahlu/mockingbird",
      "id": 7536438,
      "name": "noahlu/mockingbird"
    },
    "public": true,
    "payload": {
      "description": "blog system",
      "master_branch": "master",
      "ref": "master",
      "ref_type": "branch"
    },
    "id": "1652857721"
  },
  {
    "type": "ForkEvent",
    "created_at": "2013-01-10T07:58:29Z",
    "actor": {
      "gravatar_id": "053e38be1bd8b18bf8b1c26e11a797ff",
      "login": "rtlong",
      "avatar_url": "https://secure.gravatar.com/avatar/053e38be1bd8b18bf8b1c26e11a797ff?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/rtlong",
      "id": 199912
    },
    "repo": {
      "url": "https://api.github.com/repos/Bluebie/digiusb.rb",
      "id": 7270403,
      "name": "Bluebie/digiusb.rb"
    },
    "public": true,
    "payload": {
      "forkee": {
        "description": "A little ruby thing for talking to digiusb, like a serial port or like a telnet (whichever!)",
        "fork": true,
        "url": "https://api.github.com/repos/rtlong/digiusb.rb",
        "language": "Ruby",
        "stargazers_url": "https://api.github.com/repos/rtlong/digiusb.rb/stargazers",
        "clone_url": "https://github.com/rtlong/digiusb.rb.git",
        "tags_url": "https://api.github.com/repos/rtlong/digiusb.rb/tags{/tag}",
        "full_name": "rtlong/digiusb.rb",
        "merges_url": "https://api.github.com/repos/rtlong/digiusb.rb/merges",
        "forks": 0,
        "private": false,
        "git_refs_url": "https://api.github.com/repos/rtlong/digiusb.rb/git/refs{/sha}",
        "archive_url": "https://api.github.com/repos/rtlong/digiusb.rb/{archive_format}{/ref}",
        "collaborators_url": "https://api.github.com/repos/rtlong/digiusb.rb/collaborators{/collaborator}",
        "owner": {
          "url": "https://api.github.com/users/rtlong",
          "gists_url": "https://api.github.com/users/rtlong/gists{/gist_id}",
          "gravatar_id": "053e38be1bd8b18bf8b1c26e11a797ff",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/053e38be1bd8b18bf8b1c26e11a797ff?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/rtlong/subscriptions",
          "organizations_url": "https://api.github.com/users/rtlong/orgs",
          "received_events_url": "https://api.github.com/users/rtlong/received_events",
          "repos_url": "https://api.github.com/users/rtlong/repos",
          "login": "rtlong",
          "id": 199912,
          "starred_url": "https://api.github.com/users/rtlong/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/rtlong/events{/privacy}",
          "followers_url": "https://api.github.com/users/rtlong/followers",
          "following_url": "https://api.github.com/users/rtlong/following"
        },
        "languages_url": "https://api.github.com/repos/rtlong/digiusb.rb/languages",
        "trees_url": "https://api.github.com/repos/rtlong/digiusb.rb/git/trees{/sha}",
        "labels_url": "https://api.github.com/repos/rtlong/digiusb.rb/labels{/name}",
        "html_url": "https://github.com/rtlong/digiusb.rb",
        "pushed_at": "2013-01-08T13:23:08Z",
        "created_at": "2013-01-10T07:58:28Z",
        "has_issues": false,
        "forks_url": "https://api.github.com/repos/rtlong/digiusb.rb/forks",
        "branches_url": "https://api.github.com/repos/rtlong/digiusb.rb/branches{/branch}",
        "commits_url": "https://api.github.com/repos/rtlong/digiusb.rb/commits{/sha}",
        "notifications_url": "https://api.github.com/repos/rtlong/digiusb.rb/notifications{?since,all,participating}",
        "open_issues": 0,
        "contents_url": "https://api.github.com/repos/rtlong/digiusb.rb/contents/{+path}",
        "blobs_url": "https://api.github.com/repos/rtlong/digiusb.rb/git/blobs{/sha}",
        "issues_url": "https://api.github.com/repos/rtlong/digiusb.rb/issues{/number}",
        "compare_url": "https://api.github.com/repos/rtlong/digiusb.rb/compare/{base}...{head}",
        "issue_events_url": "https://api.github.com/repos/rtlong/digiusb.rb/issues/events{/number}",
        "name": "digiusb.rb",
        "updated_at": "2013-01-10T07:58:28Z",
        "statuses_url": "https://api.github.com/repos/rtlong/digiusb.rb/statuses/{sha}",
        "forks_count": 0,
        "assignees_url": "https://api.github.com/repos/rtlong/digiusb.rb/assignees{/user}",
        "ssh_url": "git@github.com:rtlong/digiusb.rb.git",
        "public": true,
        "has_wiki": true,
        "subscribers_url": "https://api.github.com/repos/rtlong/digiusb.rb/subscribers",
        "mirror_url": null,
        "watchers_count": 0,
        "id": 7536836,
        "has_downloads": true,
        "git_commits_url": "https://api.github.com/repos/rtlong/digiusb.rb/git/commits{/sha}",
        "downloads_url": "https://api.github.com/repos/rtlong/digiusb.rb/downloads",
        "pulls_url": "https://api.github.com/repos/rtlong/digiusb.rb/pulls{/number}",
        "homepage": null,
        "issue_comment_url": "https://api.github.com/repos/rtlong/digiusb.rb/issues/comments/{number}",
        "hooks_url": "https://api.github.com/repos/rtlong/digiusb.rb/hooks",
        "subscription_url": "https://api.github.com/repos/rtlong/digiusb.rb/subscription",
        "milestones_url": "https://api.github.com/repos/rtlong/digiusb.rb/milestones{/number}",
        "svn_url": "https://github.com/rtlong/digiusb.rb",
        "events_url": "https://api.github.com/repos/rtlong/digiusb.rb/events",
        "git_tags_url": "https://api.github.com/repos/rtlong/digiusb.rb/git/tags{/sha}",
        "teams_url": "https://api.github.com/repos/rtlong/digiusb.rb/teams",
        "comments_url": "https://api.github.com/repos/rtlong/digiusb.rb/comments{/number}",
        "open_issues_count": 0,
        "keys_url": "https://api.github.com/repos/rtlong/digiusb.rb/keys{/key_id}",
        "git_url": "git://github.com/rtlong/digiusb.rb.git",
        "contributors_url": "https://api.github.com/repos/rtlong/digiusb.rb/contributors",
        "size": 280,
        "watchers": 0
      }
    },
    "id": "1652857715"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:29Z",
    "actor": {
      "gravatar_id": "7641a96810be55debc2a1515ff0b6c2a",
      "login": "Armaklan",
      "avatar_url": "https://secure.gravatar.com/avatar/7641a96810be55debc2a1515ff0b6c2a?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/Armaklan",
      "id": 2310432
    },
    "repo": {
      "url": "https://api.github.com/repos/scrooloose/syntastic",
      "id": 248523,
      "name": "scrooloose/syntastic"
    },
    "public": true,
    "payload": {
      "action": "started"
    },
    "id": "1652857714"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:28Z",
    "actor": {
      "gravatar_id": "3c4478e6ae6c60b73d21c9fa0d1785ea",
      "login": "ChrisMissal",
      "avatar_url": "https://secure.gravatar.com/avatar/3c4478e6ae6c60b73d21c9fa0d1785ea?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/ChrisMissal",
      "id": 67798
    },
    "repo": {
      "url": "https://api.github.com/repos/ChrisMissal/NugetStatus",
      "id": 3873737,
      "name": "ChrisMissal/NugetStatus"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/ChrisMissal/NugetStatus/commits/458203e8a5b2aea9fc71041bd82b5ee2df5324cd",
          "message": "added images for build status",
          "distinct": true,
          "sha": "458203e8a5b2aea9fc71041bd82b5ee2df5324cd",
          "author": {
            "email": "chris.missal@gmail.com",
            "name": "Chris Missal"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107891,
      "before": "b04e7f47bf97821ba0b1d71da6ed82b8eb22a435",
      "head": "458203e8a5b2aea9fc71041bd82b5ee2df5324cd",
      "size": 1
    },
    "id": "1652857713"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:27Z",
    "actor": {
      "gravatar_id": "f8b3de3c77bce8a6b65841936fefe353",
      "login": "markpiro",
      "avatar_url": "https://secure.gravatar.com/avatar/f8b3de3c77bce8a6b65841936fefe353?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/markpiro",
      "id": 362803
    },
    "repo": {
      "url": "https://api.github.com/repos/markpiro/muzicbaux",
      "id": 7496715,
      "name": "markpiro/muzicbaux"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/markpiro/muzicbaux/commits/bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
          "message": "auth callback change",
          "distinct": false,
          "sha": "bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
          "author": {
            "email": "justbanter@gmail.com",
            "name": "mark"
          }
        }
      ],
      "distinct_size": 0,
      "ref": "refs/heads/gh-pages",
      "push_id": 134107890,
      "head": "bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
      "before": "b06a8ac52ce1e0a984c79a7a0d8e7a96e6674615",
      "size": 1
    },
    "id": "1652857711"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:27Z",
    "actor": {
      "gravatar_id": "8001f021514ae09142731a7d00190512",
      "login": "tmaybe",
      "avatar_url": "https://secure.gravatar.com/avatar/8001f021514ae09142731a7d00190512?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/tmaybe",
      "id": 546665
    },
    "repo": {
      "url": "https://api.github.com/repos/ubuwaits/beautiful-web-type",
      "id": 3159966,
      "name": "ubuwaits/beautiful-web-type"
    },
    "public": true,
    "payload": {
      "action": "started"
    },
    "id": "1652857705"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:26Z",
    "actor": {
      "gravatar_id": "6674c7cd4753478f1dddec7d3ca479e0",
      "login": "neeckeloo",
      "avatar_url": "https://secure.gravatar.com/avatar/6674c7cd4753478f1dddec7d3ca479e0?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/neeckeloo",
      "id": 1768645
    },
    "repo": {
      "url": "https://api.github.com/repos/pmsipilot/jquery-highchartTable-plugin",
      "id": 2986393,
      "name": "pmsipilot/jquery-highchartTable-plugin"
    },
    "public": true,
    "org": {
      "gravatar_id": "6f61ed1e3396060275238bd85c6bce45",
      "login": "pmsipilot",
      "avatar_url": "https://secure.gravatar.com/avatar/6f61ed1e3396060275238bd85c6bce45?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/pmsipilot",
      "id": 1233777
    },
    "payload": {
      "action": "started"
    },
    "id": "1652857702"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:26Z",
    "actor": {
      "gravatar_id": "404c21b3964401b264bc0ccda001c8b5",
      "login": "xyzgentoo",
      "avatar_url": "https://secure.gravatar.com/avatar/404c21b3964401b264bc0ccda001c8b5?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/xyzgentoo",
      "id": 503440
    },
    "repo": {
      "url": "https://api.github.com/repos/takashisite/TSPopover",
      "id": 4324360,
      "name": "takashisite/TSPopover"
    },
    "public": true,
    "payload": {
      "action": "started"
    },
    "id": "1652857701"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:23Z",
    "actor": {
      "gravatar_id": "34b251cf082c202fb3160b1afb810001",
      "login": "janodvarko",
      "avatar_url": "https://secure.gravatar.com/avatar/34b251cf082c202fb3160b1afb810001?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/janodvarko",
      "id": 37785
    },
    "repo": {
      "url": "https://api.github.com/repos/firebug/firebug",
      "id": 900208,
      "name": "firebug/firebug"
    },
    "public": true,
    "org": {
      "gravatar_id": "22b746e34a570b90788b575588c4ce3e",
      "login": "firebug",
      "avatar_url": "https://secure.gravatar.com/avatar/22b746e34a570b90788b575588c4ce3e?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/firebug",
      "id": 386750
    },
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/firebug/firebug/commits/2ce302eb2f4cf52963cdf0208a39193fc6f965a7",
          "message": "FBTest: move script/4932/ test into the main test list",
          "distinct": true,
          "sha": "2ce302eb2f4cf52963cdf0208a39193fc6f965a7",
          "author": {
            "email": "odvarko@gmail.com",
            "name": "Jan Odvarko"
          }
        },
        {
          "url": "https://api.github.com/repos/firebug/firebug/commits/30bbd75152df3069435f2f02d140962f1b880653",
          "message": "Merge branch 'master' of github.com:firebug/firebug",
          "distinct": true,
          "sha": "30bbd75152df3069435f2f02d140962f1b880653",
          "author": {
            "email": "odvarko@gmail.com",
            "name": "Jan Odvarko"
          }
        }
      ],
      "distinct_size": 2,
      "ref": "refs/heads/master",
      "push_id": 134107888,
      "head": "30bbd75152df3069435f2f02d140962f1b880653",
      "before": "bc2ea3c0978a178828b1dfa9229484f9b7ccb95e",
      "size": 2
    },
    "id": "1652857699"
  },
  {
    "type": "IssueCommentEvent",
    "created_at": "2013-01-10T07:58:23Z",
    "actor": {
      "gravatar_id": "29f82ebe1801087f04de6aaae92e19ea",
      "login": "pat",
      "avatar_url": "https://secure.gravatar.com/avatar/29f82ebe1801087f04de6aaae92e19ea?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/pat",
      "id": 4183
    },
    "repo": {
      "url": "https://api.github.com/repos/pat/thinking-sphinx",
      "id": 9525,
      "name": "pat/thinking-sphinx"
    },
    "public": true,
    "payload": {
      "issue": {
        "user": {
          "url": "https://api.github.com/users/lephyrius",
          "gists_url": "https://api.github.com/users/lephyrius/gists{/gist_id}",
          "gravatar_id": "ed0c8be5f65a0b1b77b9db7991f4ede1",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/ed0c8be5f65a0b1b77b9db7991f4ede1?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/lephyrius/subscriptions",
          "received_events_url": "https://api.github.com/users/lephyrius/received_events",
          "organizations_url": "https://api.github.com/users/lephyrius/orgs",
          "repos_url": "https://api.github.com/users/lephyrius/repos",
          "login": "lephyrius",
          "id": 747215,
          "starred_url": "https://api.github.com/users/lephyrius/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/lephyrius/events{/privacy}",
          "followers_url": "https://api.github.com/users/lephyrius/followers",
          "following_url": "https://api.github.com/users/lephyrius/following"
        },
        "url": "https://api.github.com/repos/pat/thinking-sphinx/issues/415",
        "labels": [

        ],
        "html_url": "https://github.com/pat/thinking-sphinx/issues/415",
        "labels_url": "https://api.github.com/repos/pat/thinking-sphinx/issues/415/labels{/name}",
        "pull_request": {
          "html_url": null,
          "patch_url": null,
          "diff_url": null
        },
        "created_at": "2013-01-05T11:13:24Z",
        "closed_at": "2013-01-05T17:28:50Z",
        "milestone": null,
        "title": "Migrating to TS3 got some error",
        "body": "```\r\nundefined method `<<' for nil:NilClass\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/active_record/sql_source.rb:107:in `block in prepare_for_render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/active_record/sql_source.rb:104:in `each'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/active_record/sql_source.rb:104:in `prepare_for_render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/active_record/sql_source.rb:61:in `render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration/index.rb:29:in `block in render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration/index.rb:29:in `collect'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration/index.rb:29:in `render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/core/index.rb:48:in `render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration.rb:39:in `block in render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration.rb:39:in `collect'\r\n.rvm/gems/ruby-1.9.3-p327/gems/riddle-1.5.4/lib/riddle/configuration.rb:39:in `render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/configuration.rb:81:in `render'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/configuration.rb:87:in `block in render_to_file'\r\n.rvm/rubies/ruby-1.9.3-p327/lib/ruby/1.9.1/open-uri.rb:35:in `open'\r\n.rvm/rubies/ruby-1.9.3-p327/lib/ruby/1.9.1/open-uri.rb:35:in `open'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/configuration.rb:87:in `render_to_file'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/rake_interface.rb:4:in `configure'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/rake_interface.rb:31:in `index'\r\n.rvm/gems/ruby-1.9.3-p327/gems/thinking-sphinx-3.0.0/lib/thinking_sphinx/tasks.rb:9:in `block (2 levels) in <top (required)>'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:228:in `call'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:228:in `block in execute'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:223:in `each'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:223:in `execute'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:166:in `block in invoke_with_call_chain'\r\n.rvm/rubies/ruby-1.9.3-p327/lib/ruby/1.9.1/monitor.rb:211:in `mon_synchronize'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:159:in `invoke_with_call_chain'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:187:in `block in invoke_prerequisites'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:185:in `each'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:185:in `invoke_prerequisites'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:165:in `block in invoke_with_call_chain'\r\n.rvm/rubies/ruby-1.9.3-p327/lib/ruby/1.9.1/monitor.rb:211:in `mon_synchronize'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:159:in `invoke_with_call_chain'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/task.rb:152:in `invoke'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:143:in `invoke_task'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:101:in `block (2 levels) in top_level'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:101:in `each'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:101:in `block in top_level'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:110:in `run_with_threads'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:95:in `top_level'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:73:in `block in run'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:160:in `standard_exception_handling'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/lib/rake/application.rb:70:in `run'\r\n.rvm/gems/ruby-1.9.3-p327/gems/rake-10.0.3/bin/rake:33:in `<top (required)>'\r\n.rvm/gems/ruby-1.9.3-p327/bin/rake:23:in `load'\r\n.rvm/gems/ruby-1.9.3-p327/bin/rake:23:in `<main>'\r\nTasks: TOP => ts:rebuild => ts:index\r\n```\r\nGot this error when migrating to thinking sphinx 3.\r\n",
        "updated_at": "2013-01-10T07:58:23Z",
        "number": 415,
        "state": "closed",
        "assignee": null,
        "id": 9704821,
        "events_url": "https://api.github.com/repos/pat/thinking-sphinx/issues/415/events",
        "comments_url": "https://api.github.com/repos/pat/thinking-sphinx/issues/415/comments",
        "comments": 8
      },
      "action": "created",
      "comment": {
        "user": {
          "url": "https://api.github.com/users/pat",
          "gists_url": "https://api.github.com/users/pat/gists{/gist_id}",
          "gravatar_id": "29f82ebe1801087f04de6aaae92e19ea",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/29f82ebe1801087f04de6aaae92e19ea?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/pat/subscriptions",
          "received_events_url": "https://api.github.com/users/pat/received_events",
          "organizations_url": "https://api.github.com/users/pat/orgs",
          "repos_url": "https://api.github.com/users/pat/repos",
          "login": "pat",
          "id": 4183,
          "starred_url": "https://api.github.com/users/pat/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/pat/events{/privacy}",
          "followers_url": "https://api.github.com/users/pat/followers",
          "following_url": "https://api.github.com/users/pat/following"
        },
        "url": "https://api.github.com/repos/pat/thinking-sphinx/issues/comments/12084063",
        "issue_url": "https://api.github.com/repos/pat/thinking-sphinx/issues/9704821",
        "created_at": "2013-01-10T07:58:23Z",
        "body": "I was just wondering what the cause of the issue was.",
        "updated_at": "2013-01-10T07:58:23Z",
        "id": 12084063
      }
    },
    "id": "1652857697"
  },
  {
    "type": "IssuesEvent",
    "created_at": "2013-01-10T07:58:22Z",
    "actor": {
      "gravatar_id": "786552a84365e60df3eeec8bc339a18c",
      "login": "imsky",
      "avatar_url": "https://secure.gravatar.com/avatar/786552a84365e60df3eeec8bc339a18c?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/imsky",
      "id": 330895
    },
    "repo": {
      "url": "https://api.github.com/repos/imsky/holder",
      "id": 4641606,
      "name": "imsky/holder"
    },
    "public": true,
    "payload": {
      "issue": {
        "user": {
          "url": "https://api.github.com/users/imsky",
          "gists_url": "https://api.github.com/users/imsky/gists{/gist_id}",
          "gravatar_id": "786552a84365e60df3eeec8bc339a18c",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/786552a84365e60df3eeec8bc339a18c?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/imsky/subscriptions",
          "organizations_url": "https://api.github.com/users/imsky/orgs",
          "received_events_url": "https://api.github.com/users/imsky/received_events",
          "repos_url": "https://api.github.com/users/imsky/repos",
          "login": "imsky",
          "id": 330895,
          "starred_url": "https://api.github.com/users/imsky/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/imsky/events{/privacy}",
          "followers_url": "https://api.github.com/users/imsky/followers",
          "following_url": "https://api.github.com/users/imsky/following"
        },
        "url": "https://api.github.com/repos/imsky/holder/issues/27",
        "labels": [

        ],
        "html_url": "https://github.com/imsky/holder/issues/27",
        "labels_url": "https://api.github.com/repos/imsky/holder/issues/27/labels{/name}",
        "pull_request": {
          "html_url": null,
          "patch_url": null,
          "diff_url": null
        },
        "title": "Fix width regression on retina display",
        "created_at": "2013-01-10T07:58:22Z",
        "closed_at": null,
        "milestone": null,
        "body": "",
        "updated_at": "2013-01-10T07:58:22Z",
        "assignee": {
          "url": "https://api.github.com/users/imsky",
          "gists_url": "https://api.github.com/users/imsky/gists{/gist_id}",
          "gravatar_id": "786552a84365e60df3eeec8bc339a18c",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/786552a84365e60df3eeec8bc339a18c?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/imsky/subscriptions",
          "organizations_url": "https://api.github.com/users/imsky/orgs",
          "received_events_url": "https://api.github.com/users/imsky/received_events",
          "repos_url": "https://api.github.com/users/imsky/repos",
          "login": "imsky",
          "id": 330895,
          "starred_url": "https://api.github.com/users/imsky/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/imsky/events{/privacy}",
          "followers_url": "https://api.github.com/users/imsky/followers",
          "following_url": "https://api.github.com/users/imsky/following"
        },
        "number": 27,
        "state": "open",
        "id": 9833911,
        "events_url": "https://api.github.com/repos/imsky/holder/issues/27/events",
        "comments_url": "https://api.github.com/repos/imsky/holder/issues/27/comments",
        "comments": 0
      },
      "action": "opened"
    },
    "id": "1652857694"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:22Z",
    "actor": {
      "gravatar_id": "f0b6d83305726d6a06282a864c92ec46",
      "login": "MartinGeisse",
      "avatar_url": "https://secure.gravatar.com/avatar/f0b6d83305726d6a06282a864c92ec46?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/MartinGeisse",
      "id": 1786083
    },
    "repo": {
      "url": "https://api.github.com/repos/MartinGeisse/public",
      "id": 4472103,
      "name": "MartinGeisse/public"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/MartinGeisse/public/commits/21ab9590d5b793d84564e68dc3f7f9ce28e6d272",
          "message": "...",
          "distinct": true,
          "sha": "21ab9590d5b793d84564e68dc3f7f9ce28e6d272",
          "author": {
            "email": "geisse@Shopgates-Mac-mini-3.local",
            "name": "Martin Geisse"
          }
        },
        {
          "url": "https://api.github.com/repos/MartinGeisse/public/commits/928877011d46d807955a7894c3397d2c5307faa9",
          "message": "...",
          "distinct": true,
          "sha": "928877011d46d807955a7894c3397d2c5307faa9",
          "author": {
            "email": "geisse@Shopgates-Mac-mini-3.local",
            "name": "Martin Geisse"
          }
        }
      ],
      "distinct_size": 2,
      "ref": "refs/heads/master",
      "push_id": 134107887,
      "head": "928877011d46d807955a7894c3397d2c5307faa9",
      "before": "bdf420d834e807827bc15eb38901d2bbb31bde17",
      "size": 2
    },
    "id": "1652857692"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:22Z",
    "actor": {
      "gravatar_id": "d89b0514cf4a50d0e53c5533fbe73e83",
      "login": "mengzhuo",
      "avatar_url": "https://secure.gravatar.com/avatar/d89b0514cf4a50d0e53c5533fbe73e83?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/mengzhuo",
      "id": 885662
    },
    "repo": {
      "url": "https://api.github.com/repos/mengzhuo/personal-Vim",
      "id": 7450902,
      "name": "mengzhuo/personal-Vim"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/mengzhuo/personal-Vim/commits/689b7eba4735c494befb3367a216cb7218d92dd6",
          "message": "format vimrc",
          "distinct": true,
          "sha": "689b7eba4735c494befb3367a216cb7218d92dd6",
          "author": {
            "email": "mengzhuo1203@gmail.com",
            "name": "Meng Zhuo"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107885,
      "head": "689b7eba4735c494befb3367a216cb7218d92dd6",
      "before": "ce44e73622d6ff7b4284ada289ec350087f5c846",
      "size": 1
    },
    "id": "1652857690"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:22Z",
    "actor": {
      "gravatar_id": "7611f72ccfcc7126d82e8edbfac70267",
      "login": "mpetersen",
      "avatar_url": "https://secure.gravatar.com/avatar/7611f72ccfcc7126d82e8edbfac70267?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/mpetersen",
      "id": 50281
    },
    "repo": {
      "url": "https://api.github.com/repos/mpetersen/nelson",
      "id": 5403274,
      "name": "mpetersen/nelson"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/mpetersen/nelson/commits/621ed66f18cdf9aadf4a685d6ea6f6cbc43dac83",
          "message": "Update README.md",
          "distinct": true,
          "sha": "621ed66f18cdf9aadf4a685d6ea6f6cbc43dac83",
          "author": {
            "email": "mail@moritzpetersen.de",
            "name": "Moritz Petersen"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107879,
      "head": "621ed66f18cdf9aadf4a685d6ea6f6cbc43dac83",
      "before": "4b17c791ddbf0cb7f6bb03989555bbd9680fdade",
      "size": 1
    },
    "id": "1652857684"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:21Z",
    "actor": {
      "gravatar_id": "d41d8cd98f00b204e9800998ecf8427e",
      "login": "graudeejs",
      "avatar_url": "https://secure.gravatar.com/avatar/d41d8cd98f00b204e9800998ecf8427e?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/graudeejs",
      "id": 1020124
    },
    "repo": {
      "url": "https://api.github.com/repos/cubesystems/i18n-leaf",
      "id": 6688885,
      "name": "cubesystems/i18n-leaf"
    },
    "public": true,
    "org": {
      "gravatar_id": "e12067ce3c567fca0d089997694f9e9f",
      "login": "cubesystems",
      "avatar_url": "https://secure.gravatar.com/avatar/e12067ce3c567fca0d089997694f9e9f?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/cubesystems",
      "id": 686284
    },
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/cubesystems/i18n-leaf/commits/196a702cf97a1d9bc076c23299fc2054580e74c7",
          "message": "Fix typo, remove contributing section.... for now",
          "distinct": true,
          "sha": "196a702cf97a1d9bc076c23299fc2054580e74c7",
          "author": {
            "email": "aldis@cubesystems.lv",
            "name": "Aldis Berjoza"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107876,
      "before": "f12210994ba154c848e712c772ede5aef718786c",
      "head": "196a702cf97a1d9bc076c23299fc2054580e74c7",
      "size": 1
    },
    "id": "1652857682"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:21Z",
    "actor": {
      "gravatar_id": "d514d73311703c0c91bc1f380134567a",
      "login": "njmittet",
      "avatar_url": "https://secure.gravatar.com/avatar/d514d73311703c0c91bc1f380134567a?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/njmittet",
      "id": 655211
    },
    "repo": {
      "url": "https://api.github.com/repos/njmittet/git-test",
      "id": 6186327,
      "name": "njmittet/git-test"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/njmittet/git-test/commits/a265dd95d563a1815e4817fba43cd157f814693f",
          "message": "Added another line",
          "distinct": true,
          "sha": "a265dd95d563a1815e4817fba43cd157f814693f",
          "author": {
            "email": "njmittet@gmail.com",
            "name": "Nils Jørgen Mittet"
          }
        },
        {
          "url": "https://api.github.com/repos/njmittet/git-test/commits/d58dd1b6d201a3a3ddd55d09b529af6374297f38",
          "message": "Merge branch 'master' of github.com:njmittet/git-test\n\nConflicts:\n\tclient.txt",
          "distinct": true,
          "sha": "d58dd1b6d201a3a3ddd55d09b529af6374297f38",
          "author": {
            "email": "njmittet@gmail.com",
            "name": "Nils Jørgen Mittet"
          }
        }
      ],
      "distinct_size": 2,
      "ref": "refs/heads/master",
      "push_id": 134107874,
      "head": "d58dd1b6d201a3a3ddd55d09b529af6374297f38",
      "before": "42cc0d9567ea359340ccd602cb6fa6215ce9e961",
      "size": 2
    },
    "id": "1652857680"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:20Z",
    "actor": {
      "gravatar_id": "11cf92381d24bdea86a357cc2c6bff4e",
      "login": "demitsuri",
      "avatar_url": "https://secure.gravatar.com/avatar/11cf92381d24bdea86a357cc2c6bff4e?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/demitsuri",
      "id": 2697636
    },
    "repo": {
      "url": "https://api.github.com/repos/JohnAlbin/git-svn-migrate",
      "id": 870387,
      "name": "JohnAlbin/git-svn-migrate"
    },
    "public": true,
    "payload": {
      "action": "started"
    },
    "id": "1652857678"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:20Z",
    "actor": {
      "gravatar_id": "699eb751118c39590468040803ec21d6",
      "login": "eatienza",
      "avatar_url": "https://secure.gravatar.com/avatar/699eb751118c39590468040803ec21d6?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/eatienza",
      "id": 1743603
    },
    "repo": {
      "url": "https://api.github.com/repos/eatienza/gopack",
      "id": 7172902,
      "name": "eatienza/gopack"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/eatienza/gopack/commits/139a78b68326dfd000e24ad55e366a3deaba40ae",
          "message": "make gpk test work in subdirectories",
          "distinct": true,
          "sha": "139a78b68326dfd000e24ad55e366a3deaba40ae",
          "author": {
            "email": "eric@ericaro.net",
            "name": "Eric Atienza"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107873,
      "head": "139a78b68326dfd000e24ad55e366a3deaba40ae",
      "before": "065c2d29b6ea565fe08cc84863cafbcabc8ce6ff",
      "size": 1
    },
    "id": "1652857675"
  },
  {
    "type": "GollumEvent",
    "created_at": "2013-01-10T07:58:19Z",
    "actor": {
      "gravatar_id": "8b608e9e82cf3cda8c66cb1b08a014b3",
      "login": "greentea039",
      "avatar_url": "https://secure.gravatar.com/avatar/8b608e9e82cf3cda8c66cb1b08a014b3?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/greentea039",
      "id": 2049309
    },
    "repo": {
      "url": "https://api.github.com/repos/GaryMcNabb/HVSTAT",
      "id": 5182252,
      "name": "GaryMcNabb/HVSTAT"
    },
    "public": true,
    "payload": {
      "pages": [
        {
          "page_name": "Home",
          "html_url": "https://github.com/GaryMcNabb/HVSTAT/wiki/Home",
          "title": "Home",
          "sha": "3753f109634c7f5ba6f465f65b5c8f575054f9f8",
          "summary": null,
          "action": "edited"
        }
      ]
    },
    "id": "1652857670"
  },
  {
    "type": "WatchEvent",
    "created_at": "2013-01-10T07:58:18Z",
    "actor": {
      "gravatar_id": "be73a0d3304f2a2c43b0c27a79045b69",
      "login": "henter",
      "avatar_url": "https://secure.gravatar.com/avatar/be73a0d3304f2a2c43b0c27a79045b69?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/henter",
      "id": 239970
    },
    "repo": {
      "url": "https://api.github.com/repos/jackyz/pobi",
      "id": 7216584,
      "name": "jackyz/pobi"
    },
    "public": true,
    "payload": {
      "action": "started"
    },
    "id": "1652857669"
  },
  {
    "type": "CreateEvent",
    "created_at": "2013-01-10T07:58:18Z",
    "actor": {
      "gravatar_id": "9721138b1cd172d47e88cd4d11614362",
      "login": "marciohariki",
      "avatar_url": "https://secure.gravatar.com/avatar/9721138b1cd172d47e88cd4d11614362?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/marciohariki",
      "id": 478795
    },
    "repo": {
      "url": "https://api.github.com/repos/marciohariki/faraja",
      "id": 7536835,
      "name": "marciohariki/faraja"
    },
    "public": true,
    "payload": {
      "description": "",
      "master_branch": "master",
      "ref": null,
      "ref_type": "repository"
    },
    "id": "1652857668"
  },
  {
    "type": "CreateEvent",
    "created_at": "2013-01-10T07:58:18Z",
    "actor": {
      "gravatar_id": "16044a3433095bc94514e1ac58a005a8",
      "login": "OdyX",
      "avatar_url": "https://secure.gravatar.com/avatar/16044a3433095bc94514e1ac58a005a8?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/OdyX",
      "id": 417403
    },
    "repo": {
      "url": "https://api.github.com/repos/OdyX/colobot-level-i18n-infra",
      "id": 7536834,
      "name": "OdyX/colobot-level-i18n-infra"
    },
    "public": true,
    "payload": {
      "description": "Translation infrastructure work for colobot levels",
      "master_branch": "master",
      "ref": null,
      "ref_type": "repository"
    },
    "id": "1652857667"
  },
  {
    "type": "IssueCommentEvent",
    "created_at": "2013-01-10T07:58:17Z",
    "actor": {
      "gravatar_id": "29bc0a400b55eb59e811fce93477ca38",
      "login": "rosenkrieger",
      "avatar_url": "https://secure.gravatar.com/avatar/29bc0a400b55eb59e811fce93477ca38?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/rosenkrieger",
      "id": 2276814
    },
    "repo": {
      "url": "https://api.github.com/repos/SynoCommunity/spksrc",
      "id": 2565137,
      "name": "SynoCommunity/spksrc"
    },
    "public": true,
    "org": {
      "gravatar_id": "35084cec3ea4e7ea80951078d2030339",
      "login": "SynoCommunity",
      "avatar_url": "https://secure.gravatar.com/avatar/35084cec3ea4e7ea80951078d2030339?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/SynoCommunity",
      "id": 1123581
    },
    "payload": {
      "issue": {
        "user": {
          "url": "https://api.github.com/users/G1zm0",
          "gists_url": "https://api.github.com/users/G1zm0/gists{/gist_id}",
          "gravatar_id": "0d0b73f1ccb919689faa904c2c658d3b",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/0d0b73f1ccb919689faa904c2c658d3b?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/G1zm0/subscriptions",
          "received_events_url": "https://api.github.com/users/G1zm0/received_events",
          "organizations_url": "https://api.github.com/users/G1zm0/orgs",
          "repos_url": "https://api.github.com/users/G1zm0/repos",
          "login": "G1zm0",
          "id": 1174845,
          "starred_url": "https://api.github.com/users/G1zm0/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/G1zm0/events{/privacy}",
          "followers_url": "https://api.github.com/users/G1zm0/followers",
          "following_url": "https://api.github.com/users/G1zm0/following"
        },
        "url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/249",
        "labels": [

        ],
        "html_url": "https://github.com/SynoCommunity/spksrc/issues/249",
        "labels_url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/249/labels{/name}",
        "pull_request": {
          "html_url": null,
          "patch_url": null,
          "diff_url": null
        },
        "title": "Newznab install DS211J",
        "created_at": "2012-09-23T14:21:36Z",
        "closed_at": null,
        "milestone": null,
        "body": "This is related to [#227](https://github.com/SynoCommunity/spksrc/issues/227#issuecomment-8543626), to keep a log.\r\nAnd ask for help if needed.\r\n\r\nStep 1 to install is setting al [Prerequisites](http://newznab.readthedocs.org/en/latest/install/#prerequisites)\r\n- Enable php and webstation in DSM.\r\n- Do NOT enable register_globals\r\n- add :/opt/share/pear to php open_basedir\r\n- ipkg install php-curl\r\n- ipkg install php-pear\r\n- edit php.ini \r\n1. (set max php memory limit to 256), won't know if this will work.\r\n2. add pear to openbase_dir (include_path = \".:/php/includes:/opt/share/pear\")\r\n\r\n[Result:](https://dl.dropbox.com/u/16935152/newsnab/step1.png)\r\n\r\n\r\n",
        "updated_at": "2013-01-10T07:58:17Z",
        "assignee": null,
        "number": 249,
        "state": "open",
        "id": 7071528,
        "events_url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/249/events",
        "comments_url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/249/comments",
        "comments": 146
      },
      "action": "created",
      "comment": {
        "user": {
          "url": "https://api.github.com/users/rosenkrieger",
          "gists_url": "https://api.github.com/users/rosenkrieger/gists{/gist_id}",
          "gravatar_id": "29bc0a400b55eb59e811fce93477ca38",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/29bc0a400b55eb59e811fce93477ca38?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/rosenkrieger/subscriptions",
          "received_events_url": "https://api.github.com/users/rosenkrieger/received_events",
          "organizations_url": "https://api.github.com/users/rosenkrieger/orgs",
          "repos_url": "https://api.github.com/users/rosenkrieger/repos",
          "login": "rosenkrieger",
          "id": 2276814,
          "starred_url": "https://api.github.com/users/rosenkrieger/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/rosenkrieger/events{/privacy}",
          "followers_url": "https://api.github.com/users/rosenkrieger/followers",
          "following_url": "https://api.github.com/users/rosenkrieger/following"
        },
        "url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/comments/12084060",
        "issue_url": "https://api.github.com/repos/SynoCommunity/spksrc/issues/7071528",
        "created_at": "2013-01-10T07:58:16Z",
        "body": "Me. Again ;-)\r\n\r\nHopefully someone who understands REGEX can help me.\r\n\r\nSo I added an ebook group, for some reason there are also movies from a certain poster in there - which I do NOT want.\r\n\r\nThe REGEX /^(?P<name>.*)$/i finds everything in the group, including the stuff i do not want. \r\n\r\nHow would I have to change it so that stuff from usenet-space-cowboys is NOT included?",
        "updated_at": "2013-01-10T07:58:16Z",
        "id": 12084060
      }
    },
    "id": "1652857665"
  },
  {
    "type": "ForkEvent",
    "created_at": "2013-01-10T07:58:17Z",
    "actor": {
      "gravatar_id": "57a77579176a45583682e372067e8d23",
      "login": "slwchs",
      "avatar_url": "https://secure.gravatar.com/avatar/57a77579176a45583682e372067e8d23?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/slwchs",
      "id": 1146116
    },
    "repo": {
      "url": "https://api.github.com/repos/DeNADev/HandlerSocket-Plugin-for-MySQL",
      "id": 837872,
      "name": "DeNADev/HandlerSocket-Plugin-for-MySQL"
    },
    "public": true,
    "org": {
      "gravatar_id": "a5c6d5d74b64284fcb7d963ee2d3cfc5",
      "login": "DeNADev",
      "avatar_url": "https://secure.gravatar.com/avatar/a5c6d5d74b64284fcb7d963ee2d3cfc5?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/DeNADev",
      "id": 1357586
    },
    "payload": {
      "forkee": {
        "description": "",
        "fork": true,
        "url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL",
        "language": "C++",
        "stargazers_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/stargazers",
        "clone_url": "https://github.com/slwchs/HandlerSocket-Plugin-for-MySQL.git",
        "tags_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/tags{/tag}",
        "full_name": "slwchs/HandlerSocket-Plugin-for-MySQL",
        "merges_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/merges",
        "forks": 0,
        "private": false,
        "git_refs_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/git/refs{/sha}",
        "archive_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/{archive_format}{/ref}",
        "collaborators_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/collaborators{/collaborator}",
        "owner": {
          "url": "https://api.github.com/users/slwchs",
          "gists_url": "https://api.github.com/users/slwchs/gists{/gist_id}",
          "gravatar_id": "57a77579176a45583682e372067e8d23",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/57a77579176a45583682e372067e8d23?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/slwchs/subscriptions",
          "organizations_url": "https://api.github.com/users/slwchs/orgs",
          "received_events_url": "https://api.github.com/users/slwchs/received_events",
          "repos_url": "https://api.github.com/users/slwchs/repos",
          "login": "slwchs",
          "id": 1146116,
          "starred_url": "https://api.github.com/users/slwchs/starred{/owner}{/repo}",
          "events_url": "https://api.github.com/users/slwchs/events{/privacy}",
          "followers_url": "https://api.github.com/users/slwchs/followers",
          "following_url": "https://api.github.com/users/slwchs/following"
        },
        "languages_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/languages",
        "trees_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/git/trees{/sha}",
        "labels_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/labels{/name}",
        "html_url": "https://github.com/slwchs/HandlerSocket-Plugin-for-MySQL",
        "pushed_at": "2012-07-10T06:30:41Z",
        "created_at": "2013-01-10T07:58:16Z",
        "has_issues": false,
        "forks_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/forks",
        "branches_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/branches{/branch}",
        "commits_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/commits{/sha}",
        "notifications_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/notifications{?since,all,participating}",
        "open_issues": 0,
        "contents_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/contents/{+path}",
        "blobs_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/git/blobs{/sha}",
        "issues_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/issues{/number}",
        "compare_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/compare/{base}...{head}",
        "issue_events_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/issues/events{/number}",
        "name": "HandlerSocket-Plugin-for-MySQL",
        "updated_at": "2013-01-10T07:58:16Z",
        "statuses_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/statuses/{sha}",
        "forks_count": 0,
        "assignees_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/assignees{/user}",
        "ssh_url": "git@github.com:slwchs/HandlerSocket-Plugin-for-MySQL.git",
        "public": true,
        "has_wiki": true,
        "subscribers_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/subscribers",
        "mirror_url": null,
        "watchers_count": 0,
        "id": 7536833,
        "has_downloads": true,
        "git_commits_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/git/commits{/sha}",
        "downloads_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/downloads",
        "pulls_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/pulls{/number}",
        "homepage": "",
        "issue_comment_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/issues/comments/{number}",
        "hooks_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/hooks",
        "subscription_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/subscription",
        "milestones_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/milestones{/number}",
        "svn_url": "https://github.com/slwchs/HandlerSocket-Plugin-for-MySQL",
        "events_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/events",
        "git_tags_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/git/tags{/sha}",
        "teams_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/teams",
        "comments_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/comments{/number}",
        "open_issues_count": 0,
        "keys_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/keys{/key_id}",
        "git_url": "git://github.com/slwchs/HandlerSocket-Plugin-for-MySQL.git",
        "contributors_url": "https://api.github.com/repos/slwchs/HandlerSocket-Plugin-for-MySQL/contributors",
        "size": 204,
        "watchers": 0
      }
    },
    "id": "1652857660"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:16Z",
    "actor": {
      "gravatar_id": "f8b3de3c77bce8a6b65841936fefe353",
      "login": "markpiro",
      "avatar_url": "https://secure.gravatar.com/avatar/f8b3de3c77bce8a6b65841936fefe353?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/markpiro",
      "id": 362803
    },
    "repo": {
      "url": "https://api.github.com/repos/markpiro/muzicbaux",
      "id": 7496715,
      "name": "markpiro/muzicbaux"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/markpiro/muzicbaux/commits/bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
          "message": "auth callback change",
          "distinct": true,
          "sha": "bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
          "author": {
            "email": "justbanter@gmail.com",
            "name": "mark"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107864,
      "head": "bbbb56de64cb3c7c1d174546fb4e340c75bb8c0c",
      "before": "b06a8ac52ce1e0a984c79a7a0d8e7a96e6674615",
      "size": 1
    },
    "id": "1652857654"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:15Z",
    "actor": {
      "gravatar_id": "b9fc678d24d871a8505c7da255993bc5",
      "login": "skorks",
      "avatar_url": "https://secure.gravatar.com/avatar/b9fc678d24d871a8505c7da255993bc5?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/skorks",
      "id": 109413
    },
    "repo": {
      "url": "https://api.github.com/repos/skorks/escort",
      "id": 7437220,
      "name": "skorks/escort"
    },
    "public": true,
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/skorks/escort/commits/047f85ba0a47de5debdb43f62c3782543e228250",
          "message": "Better formatters for help text",
          "distinct": true,
          "sha": "047f85ba0a47de5debdb43f62c3782543e228250",
          "author": {
            "email": "alan@skorks.com",
            "name": "Alan Skorkin"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/master",
      "push_id": 134107863,
      "head": "047f85ba0a47de5debdb43f62c3782543e228250",
      "before": "267ba05093edd0ea70e5dc3801a6f06b171a07d0",
      "size": 1
    },
    "id": "1652857652"
  },
  {
    "type": "PushEvent",
    "created_at": "2013-01-10T07:58:14Z",
    "actor": {
      "gravatar_id": "6a0503c195f533b5cd6bd4ccdd6cdc3e",
      "login": "kmaehashi",
      "avatar_url": "https://secure.gravatar.com/avatar/6a0503c195f533b5cd6bd4ccdd6cdc3e?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/kmaehashi",
      "id": 939877
    },
    "repo": {
      "url": "https://api.github.com/repos/jubatus/website",
      "id": 2644458,
      "name": "jubatus/website"
    },
    "public": true,
    "org": {
      "gravatar_id": "e48bca9362833c506303719cf6f2fd9c",
      "login": "jubatus",
      "avatar_url": "https://secure.gravatar.com/avatar/e48bca9362833c506303719cf6f2fd9c?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-org-420.png",
      "url": "https://api.github.com/orgs/jubatus",
      "id": 740604
    },
    "payload": {
      "commits": [
        {
          "url": "https://api.github.com/repos/jubatus/website/commits/210ed738f81eadeaf7135c7ff1b7c471d9a91312",
          "message": "fix dead link",
          "distinct": true,
          "sha": "210ed738f81eadeaf7135c7ff1b7c471d9a91312",
          "author": {
            "email": "webmaster@kenichimaehashi.com",
            "name": "Kenichi Maehashi"
          }
        }
      ],
      "distinct_size": 1,
      "ref": "refs/heads/develop",
      "push_id": 134107860,
      "before": "27b48c300994293b38c1b1d7f8cf656a551aa16f",
      "head": "210ed738f81eadeaf7135c7ff1b7c471d9a91312",
      "size": 1
    },
    "id": "1652857648"
  },
  {
    "type": "GollumEvent",
    "created_at": "2013-01-10T07:58:15Z",
    "actor": {
      "gravatar_id": "923326b259c68209a76497ad1d3ceec0",
      "login": "akrillo89",
      "avatar_url": "https://secure.gravatar.com/avatar/923326b259c68209a76497ad1d3ceec0?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/akrillo89",
      "id": 2676770
    },
    "repo": {
      "url": "https://api.github.com/repos/arsenij-solovjev/sonar-modelbus-plugin",
      "id": 6535088,
      "name": "arsenij-solovjev/sonar-modelbus-plugin"
    },
    "public": true,
    "payload": {
      "pages": [
        {
          "page_name": "Sonar Plugin Development",
          "html_url": "https://github.com/arsenij-solovjev/sonar-modelbus-plugin/wiki/Sonar-Plugin-Development",
          "title": "Sonar Plugin Development",
          "sha": "a824c9c9bb5e874e0d91809512d42654d46f8c14",
          "summary": null,
          "action": "edited"
        }
      ]
    },
    "id": "1652857651"
  },
  {
    "type": "ForkEvent",
    "created_at": "2013-01-10T07:58:13Z",
    "actor": {
      "gravatar_id": "28f08154fd59530479209fef41f674e1",
      "login": "vcovito",
      "avatar_url": "https://secure.gravatar.com/avatar/28f08154fd59530479209fef41f674e1?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
      "url": "https://api.github.com/users/vcovito",
      "id": 1354081
    },
    "repo": {
      "url": "https://api.github.com/repos/wang-bin/QtAV",
      "id": 6435042,
      "name": "wang-bin/QtAV"
    },
    "public": true,
    "payload": {
      "forkee": {
        "full_name": "vcovito/QtAV",
        "stargazers_url": "https://api.github.com/repos/vcovito/QtAV/stargazers",
        "clone_url": "https://github.com/vcovito/QtAV.git",
        "fork": true,
        "url": "https://api.github.com/repos/vcovito/QtAV",
        "tags_url": "https://api.github.com/repos/vcovito/QtAV/tags{/tag}",
        "description": "A media library based on Qt and FFmpeg. Development files for MinGW can be downloaded frome https://sourceforge.net/projects/qtav/files/?",
        "merges_url": "https://api.github.com/repos/vcovito/QtAV/merges",
        "forks": 0,
        "language": "C++",
        "private": false,
        "archive_url": "https://api.github.com/repos/vcovito/QtAV/{archive_format}{/ref}",
        "collaborators_url": "https://api.github.com/repos/vcovito/QtAV/collaborators{/collaborator}",
        "languages_url": "https://api.github.com/repos/vcovito/QtAV/languages",
        "owner": {
          "gists_url": "https://api.github.com/users/vcovito/gists{/gist_id}",
          "gravatar_id": "28f08154fd59530479209fef41f674e1",
          "url": "https://api.github.com/users/vcovito",
          "type": "User",
          "avatar_url": "https://secure.gravatar.com/avatar/28f08154fd59530479209fef41f674e1?d=https://a248.e.akamai.net/assets.github.com%2Fimages%2Fgravatars%2Fgravatar-user-420.png",
          "subscriptions_url": "https://api.github.com/users/vcovito/subscriptions",
          "organizations_url": "https://api.github.com/users/vcovito/orgs",
          "received_events_url": "https://api.github.com/users/vcovito/received_events",
          "repos_url": "https://api.github.com/users/vcovito/repos",
          "login": "vcovito",
          "starred_url": "https://api.github.com/users/vcovito/starred{/owner}{/repo}",
          "id": 1354081,
          "events_url": "https://api.github.com/users/vcovito/events{/privacy}",
          "followers_url": "https://api.github.com/users/vcovito/followers",
          "following_url": "https://api.github.com/users/vcovito/following"
        },
        "git_refs_url": "https://api.github.com/repos/vcovito/QtAV/git/refs{/sha}",
        "labels_url": "https://api.github.com/repos/vcovito/QtAV/labels{/name}",
        "pushed_at": "2013-01-07T12:17:03Z",
        "html_url": "https://github.com/vcovito/QtAV",
        "trees_url": "https://api.github.com/repos/vcovito/QtAV/git/trees{/sha}",
        "forks_url": "https://api.github.com/repos/vcovito/QtAV/forks",
        "commits_url": "https://api.github.com/repos/vcovito/QtAV/commits{/sha}",
        "branches_url": "https://api.github.com/repos/vcovito/QtAV/branches{/branch}",
        "notifications_url": "https://api.github.com/repos/vcovito/QtAV/notifications{?since,all,participating}",
        "created_at": "2013-01-10T07:58:13Z",
        "has_issues": false,
        "blobs_url": "https://api.github.com/repos/vcovito/QtAV/git/blobs{/sha}",
        "issues_url": "https://api.github.com/repos/vcovito/QtAV/issues{/number}",
        "compare_url": "https://api.github.com/repos/vcovito/QtAV/compare/{base}...{head}",
        "open_issues": 0,
        "contents_url": "https://api.github.com/repos/vcovito/QtAV/contents/{+path}",
        "name": "QtAV",
        "statuses_url": "https://api.github.com/repos/vcovito/QtAV/statuses/{sha}",
        "assignees_url": "https://api.github.com/repos/vcovito/QtAV/assignees{/user}",
        "forks_count": 0,
        "updated_at": "2013-01-10T07:58:13Z",
        "issue_events_url": "https://api.github.com/repos/vcovito/QtAV/issues/events{/number}",
        "ssh_url": "git@github.com:vcovito/QtAV.git",
        "subscribers_url": "https://api.github.com/repos/vcovito/QtAV/subscribers",
        "mirror_url": null,
        "public": true,
        "has_wiki": true,
        "git_commits_url": "https://api.github.com/repos/vcovito/QtAV/git/commits{/sha}",
        "downloads_url": "https://api.github.com/repos/vcovito/QtAV/downloads",
        "id": 7536832,
        "pulls_url": "https://api.github.com/repos/vcovito/QtAV/pulls{/number}",
        "has_downloads": true,
        "issue_comment_url": "https://api.github.com/repos/vcovito/QtAV/issues/comments/{number}",
        "watchers_count": 0,
        "homepage": "",
        "hooks_url": "https://api.github.com/repos/vcovito/QtAV/hooks",
        "subscription_url": "https://api.github.com/repos/vcovito/QtAV/subscription",
        "milestones_url": "https://api.github.com/repos/vcovito/QtAV/milestones{/number}",
        "events_url": "https://api.github.com/repos/vcovito/QtAV/events",
        "svn_url": "https://github.com/vcovito/QtAV",
        "git_tags_url": "https://api.github.com/repos/vcovito/QtAV/git/tags{/sha}",
        "teams_url": "https://api.github.com/repos/vcovito/QtAV/teams",
        "comments_url": "https://api.github.com/repos/vcovito/QtAV/comments{/number}",
        "open_issues_count": 0,
        "keys_url": "https://api.github.com/repos/vcovito/QtAV/keys{/key_id}",
        "contributors_url": "https://api.github.com/repos/vcovito/QtAV/contributors",
        "size": 4286,
        "watchers": 0,
        "git_url": "git://github.com/vcovito/QtAV.git"
      }
    },
    "id": "1652857642"
  }
]
)EOF";

} // namespace

class iostreams_test
{
public:
    void
    run()
    {
        {
            std::ifstream src;
            error_code ec;
            auto jv = parse(src, ec);
            BOOST_TEST( jv == nullptr );
            BOOST_TEST( ec );
        }
        {
            std::istringstream src(json_text);
            auto jv = parse(src);
            BOOST_TEST(jv != nullptr);
        }
        {
            std::istringstream src(json_text);
            streambuf sb;
            std::copy(
                std::istreambuf_iterator<char>(src),
                std::istreambuf_iterator<char>(),
                std::ostreambuf_iterator<char>(&sb));
            auto jv = sb.value();
            BOOST_TEST(jv != nullptr);
        }
    }
};

TEST_SUITE(iostreams_test, "boost.json.iostreams");

BOOST_JSON_NS_END
