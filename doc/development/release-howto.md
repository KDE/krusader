# Krusader Release Howto

This is a step-by-step release guide intended to streamline release process.
Don't follow this blindly though. Think, test and improve the guide as the process changes!

Release managers are required to be a part of the Krew with a Developer role.
It implies that you set the developer access up already.


## Overall release process

Before the release:
* [Create an issue](#create-an-issue) to claim yourself as a release manager.
* Based on the discussion in the issue, set an appropriate [feature freeze](#feature-freeze) for the release and agree on the approximate release date.
* Wait for changes to be reviewed and pushed.
* In case of a major or a minor release discuss and [pick a release name](#release-name).

At least two weeks before the release:
* Decide on the release date.
* If needed, [create a release branch](#create-a-release-branch).
* Merge changes for the documentation:
  * Update or ask to [update the documentation](#update-the-documentation), including features for a major or a minor release.
  * [Update the docs and AppStream files](#code-review-documentation-and-appstream-files).
* [Declare a string and doc freeze](#declare-a-string-and-doc-freeze).

At least one week before the release:
* Prepare a merge request with changes for the release 
  * [Update change logs and news](#code-review-changelog-and-news-update) in the meantime.
  * [Update the version](#code-review-version-bump).
* Prepare a [merge request for the website](#code-review-website-update).
* [Prepare the environment](#environment) and practice if needed.

On the release date:
* [Follow the steps](#on-the-release-date) to prepare and release the package.
* [Wrap up](#wrap-up): check bugzilla and merge the changes to master.

See also [the KDE community guide for releasing software](https://community.kde.org/ReleasingSoftware).


## Create an issue

Create an [issue](https://invent.kde.org/utilities/krusader/-/issues) to claim yourself as a release manager.
Ask if there are any outstanding issues, code reviews, unpublished work that devs want to include into the release.
Wait for a week to gather replies.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/issues/22) was requested externally, so it's not an ideal example.
Check the email threads from before-GitLab era:
[v2.7.1](https://groups.google.com/d/msg/krusader-devel/a3OdfBDv4K0/b_S_uap-BgAJ),
[v2.7.0](https://groups.google.com/d/msg/krusader-devel/TMIUqY1gyRg/_Vl7J64aBAAJ).


## Feature freeze

Discuss the proposed changes in the issue, set a feature freeze for the release.
It means no new features should be included to the release later
except for the agreed upon at this stage or bug fixes.
Agree on the approximate release date.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/issues/22#note_499898),
[v2.7.1](https://groups.google.com/d/msg/krusader-devel/a3OdfBDv4K0/G2YXjGtmBwAJ),
[v2.7.0](https://groups.google.com/d/msg/krusader-devel/TMIUqY1gyRg/HVwioll8AAAJ).


## Release name

Discuss and pick a release name in case of a major or a minor release.
Bug fix releases carry the release name over (for example, v2.7.x are all "Peace of Mind").
See [the history of the release names](https://docs.kde.org/stable5/en/krusader/krusader/release_overview.html)
and don't repeat, it's not fun.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/issues/22#note_499898),
[v2.7.0](https://groups.google.com/d/msg/krusader-devel/TMIUqY1gyRg/HVwioll8AAAJ).


## Create a release branch

In case you're doing a minor or major release (i.e. 2.7 -> 2.8 or 2.8 -> 3.0), `stable` branch needs to be remapped.
(In case you're doing patch version update (i.e. 2.7.1 -> 2.7.2), no update to stable branch is needed.)

For example, you are going to do 2.7 -> 2.8. Then
1. First create a branch at the same point as `stable` to mark a past release branch. Name it `2.7`.
2. Hard reset stable to `master`
3. Create a commit in `master` updating the app version to `2.9.0-dev` (`stable` should keep previous master version which is `2.8.0-dev`, so no commit is needed)
4. Push `master`, `2.7` to the remote.
5. Since `stable` is protected from non-fast-forward updates, first delete the remote branch with `git push origin :stable`. Then push `stable` to the remote.

Now you are ready for the release.


## Update the documentation

Update or ask to update the documentation.
Check if changes made from the previous release have been documented.

Update features for a major or a minor release in the `doc/handbook/features.docbook`.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/merge_requests/108),
[v2.7.0](https://phabricator.kde.org/D12531?vs=on&id=33125).


## Code Review: Documentation and AppStream files

Propose the release date.
This should be at least 3 weeks out from current date as we need to put the date into the docs,
this change has to be reviewed, approved and merged,
and we need to allow at least 2 weeks for translators to update the strings.

Update the following files with new release version, date and name (if applicable):
* `app/org.kde.krusader.appdata.xml`
* `doc/handbook/index.docbook`
* `doc/handbook/release-overview.docbook`

**This must be pushed before doc freeze email.**

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/merge_requests/109),
[v2.7.2](https://phabricator.kde.org/D22982).


## Declare a string and doc freeze

Declare a string and doc freeze at least 2 weeks before the release date
by sending an email to kde-i18n-doc@kde.org and notifying devs on the issue.

Subject: `String and doc freeze for Krusader stable branch`

Body template:
```
Hi Localization team,

Krusader stable branch is now in the string and doc freeze mode.
We plan to release v{VERSION} on {DATE}.
Please update the translations accordingly.

Thanks,
{YOUR_NAME} on behalf of Krusader team.
```

Examples:
[v2.7.1](https://groups.google.com/forum/#!topic/krusader-devel/Fe5bOeTbCCo).


## Code Review: ChangeLog and NEWS update

Update the following files accordingly:
* `Changelog`
* `NEWS`
* `doc/ChangeLog`

Search in git log for changelog messages and add them to the `ChangeLog` file:
```
cd krusader
LAST_VERSION="vX.Y.Z"
git log $LAST_VERSION.. | grep 'FIXED:\|ADDED:\|UPDATED:\|CHANGED:\|REMOVED:' | sort
```

Add changes from bugzilla bug reports and code reviews manually if necessary.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/merge_requests/107),
[v2.7.1](https://phabricator.kde.org/D14619?id=39104),
[v2.7.0](https://phabricator.kde.org/D12818?id=33973).


## Code Review: Version bump

Update the following files with new release version, date, name (if applicable):
* `CMakeLists.txt`
* `README`

**Verify that the version is building without warnings for both Debug and Release targets.
Verify it's running correctly and you don't see any obvious problems.
Do this ahead of time to be able to fix problems early!**

Commit message template:
```
Released v{X.Y.Z}

Resolves #{N}.

Discussion: {LINK}
```

Here `N` is the index of an issue about the release.
The `Resolves` tag will close the issue once you [merge](#merge-new-version-tag-to-master) the release tag
into the `master` as the last step of this process.

Examples:
[v2.8.0](https://invent.kde.org/utilities/krusader/-/merge_requests/110).


## Code Review: Website update

Update the following pages:
* `get-krusader/release-archive/index.html`
* `get-krusader/index.html`
* `release/{VERSION}/changelog.txt`
* `release/{VERSION}/release_notes.txt`

Copy release notes and changelog from NEWS and ChangeLog accordingly.

Also check if website copyright year needs to be updated.

Examples:
[v2.8.0](https://invent.kde.org/websites/krusader-org/-/merge_requests/10).


## Environment

Prepare this before the release date to be ready.
It doesn't hurt to practice [the release steps](#on-the-release-date) ahead of time.

### First time setup
```
mkdir -v krusader-release-env && cd krusader-release-env
git clone git@invent.kde.org:utilities/krusader.git
git clone git@invent.kde.org:sdk/releaseme.git
```

### Refresh
```
cd krusader-release-env
cd krusader && git switch stable && git pull ; cd ..
cd releaseme && git pull ; cd ..
```


## On the release date

### Verify the branch

Starting from version 2.7.1 we release only from the `stable` branch.
In your dev environment do
```
cd krusader && git switch stable && git pull
```

Verify that ChangeLog and NEWS update is pushed or push it.

### Commit and push version updates

Apply the version bump commit. Tag it:
```
VERSION=`cat CMakeLists.txt | grep 'set(VERSION' | awk -F '"' '{print $2; }'`
echo $VERSION  # verify the version
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

### Refresh the release environment

See [Refresh Environment](#refresh). The following steps assume you are in `krusader-release-env`.

### Create tarball

The `tarme` script from the `releaseme` repo is intended as a universal snapshot creator.
Let's run it:
```
mkdir -v tarme-wdir && cd tarme-wdir
../releaseme/tarme.rb --origin stable --version $VERSION krusader
```
This will fetch the latest code from the `stable` branch, fetch the latest localization files,
create an html report with localization status and create the tarball.

Check the report but *don't use the tarball*.
We don't need the latest from the branch as it has the commit with the `*-dev` version bump
and potentially other commits on top of it if somebody pushed to the branch after you.
Until tarme has a parameter to use a tag or a commit hash, the tarball is useless for the release purpose.

As of October 2022, localization files are injected into the `krusader` repo,
both `master` and `stable` branches.
It means the repo is self-contained and we can create the snapshot manually:

```
cd ..  # go back to krusader-release-env
mkdir -v tarball-wdir && cd tarball-wdir
git clone ../krusader
cd krusader/
git reset --hard v${VERSION}
rm -rf .git
cd ..
mv krusader krusader-${VERSION}
tar -cJf krusader-${VERSION}.tar.xz krusader-${VERSION}
```

It's worth checking the difference between the two ways we created the snapshots:
```
cd ..
diff -Naur tarme-wdir/krusader-${VERSION} tarball-wdir/krusader-${VERSION} > tarme.patch
```

Now let's clean up to avoid mistakes:
```
rm -rf tarme-wdir
cp tarball-wdir/krusader-${VERSION}.tar.xz .
```

### Test the package

**Corrupted translated documentation can break compiling, it's mandatory to do this step!**

Unpack:
```
tar -xJf krusader-${VERSION}.tar.xz
```

Build and run:
```
mkdir -v build-${VERSION} && cd build-${VERSION} && cmake -DCMAKE_INSTALL_PREFIX=../install-${VERSION} -DQT_PLUGIN_INSTALL_DIR=../install-${VERSION} ../krusader-${VERSION} && make -j8 && make install ; cd ..

./install-${VERSION}/bin/krusader
```

Go to `Help -> About Krusader` and verify the version.

### Upload the package to a KDE server

```
curl -T krusader-${VERSION}.tar.xz ftp://upload.kde.org/incoming/
```

### Create a Sysadmin Request

Send a ticket for KDE Sysadmins asking to publish the package.
The tickets are still created on Phabricator until the task system is migrated to GitLab.
To create a ticket go to [Tasks](https://phabricator.kde.org/maniphest/),
in the top right corner click `Create Task -> + New Sysadmin Request`.

Compute the hash to report:
```
sha256sum krusader-${VERSION}.tar.xz
```

Subject: `Krusader v{VERSION} release`

Priority: `Normal`

Body template:
```
Dear Sysadmins,

Please release a package for a new Krusader version to download.kde.org!

The package is uploaded to ftp://upload.kde.org/incoming/.

Destination: https://download.kde.org/stable/krusader/{VERSION}/krusader-{VERSION}.tar.xz
SHA256: {SHA256}

Please also add this release to the list of Krusader versions on bugs.kde.org.

Thanks,
{YOUR_NAME}.
```

We ask them to add versions on Bugzilla since the *editcomponents* permission is needed.
This permission allows to change settings for ALL projects.
Therefore only sysadmins can do this and nobody of the Krusader devs.

Example tickets:
[v2.8.0](https://phabricator.kde.org/T15998),
[v2.7.1](https://phabricator.kde.org/T9350).

### Update the package checksum in the website change

Earlier, when you create the CR for the website change, you didn't know the hash of the package since you haven't created a package yet. Now you know it, and it's a good time to update it while you wait for response from admins.

Example checksum updates:
[v2.8.0](https://invent.kde.org/websites/krusader-org/-/merge_requests/10#note_565720).

### Wait for the package to propagate on KDE servers

Once admins upload the package, they will close the ticket. You need to wait until you see some mirrors become available and then you can distribute the package.

### Update the Krusader website

Simply push your changes to krusader.org repository and they'll become available as soon as a recurrent deployment job discovers the change and publishes the update. May take from a few minutes to a few hours (sometimes the recurrent job is down) - usually it's very quick. If your changes are not deployed in 2 hours, create a sysadmin ticket about it.

Example merges:
[v2.8.0](https://invent.kde.org/websites/krusader-org/-/commit/db5d6d8b1f47a30d0b65fe5ea7b743a2be6ae703).

### Send a letter to mailing lists

Once the website is updated, send a letter to the following mailing lists:
* krusader-devel@googlegroups.com
* krusader-users@googlegroups.com
* krusader-news@googlegroups.com
* kde-announce@kde.org

Please get the permission to post ahead of time.

Example letters:
[v2.8.0](https://groups.google.com/g/krusader-users/c/aD6GqHUKjGM),
[v2.7.2](https://groups.google.com/g/krusader-users/c/7terZtDa8Ss).


## Wrap up

### Check if bugzilla is updated accordingly

[Click here](https://bugs.kde.org/enter_bug.cgi?product=krusader) and check if new version is present in the list.

### Merge new version tag to master

Since `stable` contains changes not present in `master` (ChangeLog and documentation changes at least),
you need to deliver them to the `master` branch.
Merge the release tag commit git into the branch:

```
git switch master
git pull
git merge --no-ff --no-commit --no-log v${VERSION}
```

You may see a lot of conflicts and changes in localization-enabled files,
such as in `po` dir and `.desktop`, `.appdata.xml` files.
This happens since the branches have separate localization injection streams.
There is a helper script that restore these files to the state before the merge,
with the exception to `.appdata.xml` that often contains version changes
and requires attention.

```
./tools/restore-l10n-files.sh
```

Resolve remaining conflicts (will be at least in `CMakeLists.txt`) keeping the Krusader version that is in `master` and stage them. Then do

```
git commit -m "Merged v${VERSION} updates"
git push
```

[Check](https://invent.kde.org/utilities/krusader/-/issues) that the issue
about the release is closed by the push
(in case you used `Resolves` tag [here](#code-review-version-bump)).
If not, close it manually.

Example merges:
[v2.8.0](c80c9ec0) + [fix](f7be3730),
[v2.7.1](8d5946a1).
