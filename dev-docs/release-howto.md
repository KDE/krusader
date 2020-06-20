# Krusader Release Howto

This is a step-by-step release guide intended to streamline release process.
Don't follow this blindly though. Think, test and improve the guide as the process changes!

Many of the steps refer to the previous development system — the Phabricator.
Please update the guide as you follow the steps for releasing the next version.


## Overall release process

* Write a letter to krusader-devel group to claim yourself as a release manager, ask if there are any outstanding issues, code reviews, unpublished work that devs want to include into the release. Wait for a week to gather replies. Examples: [v2.7.0](https://groups.google.com/d/msg/krusader-devel/TMIUqY1gyRg/_Vl7J64aBAAJ), [v2.7.1](https://groups.google.com/d/msg/krusader-devel/a3OdfBDv4K0/b_S_uap-BgAJ).
* Discuss the proposed changes, set a feature freeze for the release (no new features should be included to the release later except the agreed on at this stage or bug fixes). Agree on the approximate release date. Examples: [v2.7.0](https://groups.google.com/d/msg/krusader-devel/TMIUqY1gyRg/HVwioll8AAAJ), [v2.7.1](https://groups.google.com/d/msg/krusader-devel/a3OdfBDv4K0/G2YXjGtmBwAJ).
* Wait for changes to be reviewed and pushed.
* Discuss and pick a release name in case of major or minor release. Bug fix releases carry the release name over.
* Create a release branch if needed (see the corresponding section below).
* Update or ask to update the documentation. Check if changes made from the previous release have been documented. Update features for major or minor release (example: [v2.7.0](https://phabricator.kde.org/D12531?vs=on&id=33125)).
* Propose the release date (for example, 3 weeks from now), update the docs and AppStream files. Examples: [v2.7.2](https://phabricator.kde.org/D22982).
* Declare the string freeze and doc freeze stage for 2 weeks and notify translators. Examples: [v2.7.1](https://groups.google.com/forum/#!topic/krusader-devel/Fe5bOeTbCCo).
* Update change logs and news in the meantime. Examples: [v2.7.0](https://phabricator.kde.org/D12818?id=33973), [v2.7.1](https://phabricator.kde.org/D14619?id=39104).
* Prepare review: version update. Examples: [v2.7.1](https://phabricator.kde.org/D14621).
* Prepare review: website update. Examples: [v2.7.1](https://phabricator.kde.org/D14622).
* On the release date: prepare and release the package (see the corresponding section below).
* Wrap up: cleanup and check a few things (see the corresponding section below).


## Environment

These steps assume that you set the developer access up already.

### First time setup
```
mkdir -v krusader-release-env && cd krusader-release-env
git clone git@invent.kde.org:utilities/krusader.git
git clone git@invent.kde.org:sdk/kde-dev-scripts.git
```

### Refresh
```
cd krusader-release-env
cd krusader && git checkout stable && git pull ; cd ..
cd kde-dev-scripts && git pull ; cd ..
```


## Create a release branch

In case you're doing a minor or major release (i.e. 2.7 -> 2.8 or 2.8 -> 3.0), `stable` branch needs to be remapped.

For example, you are going to do 2.7 -> 2.8. Then
1. First create a branch at the same point as `stable` to mark a past release branch. Name it `2.7`.
2. Hard reset stable to `master`
3. Create a commit in `master` updating the app version to `2.9.0-dev` (`stable` should keep previous master version which is `2.8.0-dev`, so no commit is needed)
4. Push `master`, `2.7` and `stable` to central repo in this order.

Now you are ready for the release.

In case you're doing patch version update (i.e. 2.7.1 -> 2.7.2), no update to stable branch is needed.


## Code Review: Documentation and AppStream files

Update the following files with new release version, date, name (if applicable) and features (if applicable):
* `doc/index.docbook`
* `doc/features.docbook`
* `doc/release-overview.docbook`
* `krusader/org.kde.krusader.appdata.xml`

**This must be pushed before doc freeze email.**


## Code Review: ChangeLog and NEWS update

Update the following files accordingly:
* `Changelog`
* `NEWS`
* `doc-extras/ChangeLog`

Search in git log for changelog messages and add them to the `ChangeLog` file:
```
cd krusader
LAST_VERSION="vX.Y.Z"
git log $LAST_VERSION.. | grep 'FIXED:\|ADDED:\|UPDATED:\|CHANGED:\|REMOVED:' | sort
```

Add changes from bugzilla bug reports and code reviews manually if necessary.


## Code Review: Version bump

Update the following files with new release version, date, name (if applicable):
* `CMakeLists.txt`
* `README`

**Verify that the version is building without warnings for both Debug and Release targets.
Verify it's running correctly and you don't see any obvious problems.
Do this ahead of time to be able to fix problems early!**


## Code Review: Website update

Update the following pages:
* `index.html`
* `get-krusader/release-archive/index.html`
* `get-krusader/index.html`
* `release/{VERSION}/changelog.txt`
* `release/{VERSION}/release_notes.txt`

Copy release notes and changelog from NEWS and ChangeLog accordingly.


## On the release date

### Verify the branch

Starting from version 2.7.1 we release only from the `stable` branch.
```
cd krusader && git checkout stable && git pull
```

Verify that ChangeLog and NEWS update is pushed or push it.

### Commit and push version updates

Apply version bump commit and tag it:
```
VERSION=`cat CMakeLists.txt | grep 'set(VERSION' | awk -F '"' '{print $2; }'`
git tag -a v${VERSION} -m "Tagging krusader-${VERSION}"
```

Don't push it right away! First edit `CMakeLists.txt` to change version to development version
(for example, for v2.7.1 release put `2.7.2-dev` there). Commit. Check everything is good:
```
git log
```

Now push with tags:
```
git push --follow-tags
```

This way ensures there are no commits in between version bumps.

### Create tarball

Update `kde-dev-scripts/createtarball/config.ini` with
```
gitTag       = vX.Y.Z
version      = X.Y.Z
```
and do
```
cd ../kde-dev-scripts/createtarball/
./create_tarball_kf5.rb -n -a krusader
mv -vi krusader-$VERSION ../..
mv -vi krusader-$VERSION.tar.xz ../..
cd ../../
```

### Test the package

**Corrupted translated documentation can break compiling, it's mandatory to do this step!**

```
mkdir -v build-${VERSION} && cd build-${VERSION} && cmake -DCMAKE_INSTALL_PREFIX=../install-${VERSION} -DQT_PLUGIN_INSTALL_DIR=../install-${VERSION} ../krusader-${VERSION} && make -j 3 && make install ; cd ..

./build-${VERSION}/krusader/krusader
```

Go to `Help -> About Krusader` and verify the version.

### Upload the package to a KDE server

```
curl -T krusader-${VERSION}.tar.xz ftp://upload.kde.org/incoming/
```

### Create a Sysadmin Request

Create a new Sysadmin Request asking to publish the package.

```
sha256sum krusader-${VERSION}.tar.xz
```

Example task: [Krusader v2.7.1 release](https://phabricator.kde.org/T9350)

Task template:
```
Dear Sysadmin,

**Package pending for download.kde.org: Krusader**

New release for the Krusader project. The package is uploaded to ftp://upload.kde.org/incoming/

Desired destination: stable/krusader/{VERSION}
Full link: http://download.kde.org/stable/krusader/{VERSION}/krusader-{VERSION}.tar.xz
SHA256: {SHA256}

**Please add this release to the list of Krusader versions on bugs.kde.org.**

Thanks,
{YOUR_NAME}.
```

We ask them to add versions on Bugzilla since the *editcomponents* permission is needed.
This permission allows to change settings for ALL projects.
Therefore only sysadmins can do this and nobody of the Krusader devs.

### Update the package checksum in the website change

Earlier, when you create the CR for the website change, you didn't know the hash of the package since you haven't created a package yet. Now you know it, and it's a good time to update it while you wait for response from admins.

Example: [v2.7.1 checksum update](https://phabricator.kde.org/D14622?vs=39108&id=39491)

### Wait for the package to propagate on KDE servers

Once admins upload the package, they will close the ticket. You need to wait until you see some mirrors become available and then you can distribute the package.

### Update the Krusader website

Simply push your changes to krusader.org repository and they'll become available as soon as some recurrent job notices the change and publishes the update. May take from a few minutes to a few hours (sometimes recurrent job is down) - usually it's very quick. If your changes are not deployed in 2 hours, create a sysadmin ticket about it.

### Send a letter to mailing lists

Once the website is updated, send a letter to the following mailing lists:
* krusader-devel@googlegroups.com
* krusader-users@googlegroups.com
* krusader-news@googlegroups.com
* kde-announce-apps@kde.org

Please get the permission to post ahead of time.


## Wrap up

### Push createtarball config

The changes you made in `kde-dev-scripts` needs to be pushed to the master branch.

Example commit for [v2.7.1](https://cgit.kde.org/kde-dev-scripts.git/commit/?id=61fced5cc064d22a0706173f85cfccf4d44a9a67).

### Check if bugzilla is updated accordingly

[Click here](https://bugs.kde.org/enter_bug.cgi?product=krusader) and check if new version is present in the list.

### Merge your new version tag to master

Since `stable` contains changes not present in `master` (ChangeLog and documentation changes at least), you need to deliver them to the `master` branch. Merge your vX.Y.Z tag into the branch and resolve conflicts in places where version specified. Keep Krusader version that is in `master` during the conflict resolution.

Example merge for [v2.7.1](https://cgit.kde.org/krusader.git/commit/?id=8d5946a150c7ef3e7c9607419dd45d7945d6463f).
