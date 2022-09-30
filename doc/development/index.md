# Krusader Development Documentation

This reference will help you to get started with Krusader development.


## Krusader resources for users

* [Project homepage](https://krusader.org/)
* [Release downloads](https://download.kde.org/stable/krusader/)
* [User mailing-list](https://groups.google.com/group/krusader-users) — for user questions, tracked by maintainers, answered by users and maintainers
* [System & Utilities forum on kde.org](https://forum.kde.org/viewforum.php?f=225) — user questions rarely appear here, answered by forum community
* [Bugzilla](https://bugs.kde.org/buglist.cgi?product=krusader) — all bugs and wishes are tracked here
* [Krusader extensions](https://store.kde.org/browse/cat/370/ord/top/) — user-contributed styles and user actions
* [Krusader app page on kde.org](https://kde.org/applications/en/utilities/org.kde.krusader)


## Krusader resources for developers

* [Developer mailing-list](https://groups.google.com/group/krusader-devel) — for development discussion and questions
* [Repository](https://invent.kde.org/utilities/krusader)
* [Repository mirror on GitHub](https://github.com/KDE/krusader) — pull requests are ignored here
* [Automatic builds](https://build.kde.org/job/Extragear/job/krusader/)
* [Krazy: static code analysis](http://ebn.kde.org/krazy/reports/extragear/utils/krusader/index.html)
* [Krusader coding style](https://techbase.kde.org/Policies/Frameworks_Coding_Style) — same style as for Frameworks
* [Release Howto](release-howto.md) — step-by-step release guide
* [krusader.org website repository](https://invent.kde.org/websites/krusader-org) — the website is deployed via CI from this repo

### General KDE development resources

* [KDE developer mailing-list](https://mail.kde.org/mailman/listinfo/kde-devel)
* [KDE general announcements mailing-list](https://mail.kde.org/mailman/listinfo/kde-announce)
* [KDE applications release announcements mailing-list](https://mail.kde.org/mailman/listinfo/kde-announce-apps) — [we announce](release-howto.md#send-a-letter-to-mailing-lists) Krusader releases here
* [IRC channel #kde-devel on Libera Chat](irc://irc.libera.chat/kde-devel) — for quick questions regarding anything related to KDE development
* [Git hooks for KDE Bugzilla](https://community.kde.org/Infrastructure/Git/Hooks#Keywords) — use these keywords in your commits
* [Closing issues through commit messages](https://docs.gitlab.com/ee/user/project/issues/managing_issues.html#closing-issues) — add "Resolved #xxx" in a commit message to close a GitLab issue automatically

### Krusader dependencies

* [KIO API](https://api.kde.org/frameworks/kio/html/index.html) — KIO is heavily used for file related operations (local and remote).
* [KIO repository](https://invent.kde.org/frameworks/kio)
* [KBookmarks repository](https://invent.kde.org/frameworks/kbookmarks) — used for editing Krusader bookmarks

### Recommended IDE and developer tools

* [KDevelop IDE](http://kdevelop.org)
* [Qt Creator IDE](https://www.qt.io/ide/)
* [Cuttlefish](https://community.kde.org/Plasma/DeveloperGuide#Icon_Viewer:_Cuttlefish) — a useful icon browser for KDE

### Recommended reading

* [Qt Howto pages](https://wiki.qt.io/Category:HowTo)
* [Qt New Signal Slot Syntax](https://wiki.qt.io/New_Signal_Slot_Syntax)
* [Freedesktop icon names](https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html) — official cross-DE icon names, please try to use these first and fallback to the icon names included in Breeze to be more independent from specific icon themes


## Contributing

### Quick guide

1. Get a [KDE identity account](https://community.kde.org/Infrastructure#Identity_Accounts).
2. Fork the [Krusader repository](https://invent.kde.org/utilities/krusader).
3. Clone your fork to your dev box.
4. Make changes in your local repository in a branch.
5. Push your branch to the fork.
6. Create a merge request (aka pull request) to the main repo, mark it with "Needs Review" label.
7. Wait for devs to review and resolve all comments.
8. Once the label is changed to "Approved" and all threads are resolved, devs will merge your change to the main repo.

Find more details in [Forking workflow](https://invent.kde.org/help/user/project/repository/forking_workflow.md).

### Commit guidelines

Familiarize yourself with commit message style by reading several commit messages in the repository. Notice that for simple changes we use single commits and for multiple related changes we use branches and merge them into master with a descriptive merge commit message.

If your change fixes a bug reported on bugs.kde.org, use both the `FIXED:` keyword with the bug number in square brackets and bug title, and `BUG:` keyword that triggers the Git hook to close the bug. Refer to an example below.

Your change must be reviewed and approved before you can push it to master branch.
You must adhere to [Krusader coding style](https://techbase.kde.org/Policies/Frameworks_Coding_Style) and use the best coding practices.

Your commit (or branch merge if your change is a series of commits) must contain the code review link. It's recommended to place it on the last line and separate it from summary with an empty line. For example:

```
One-line commit title

Description of the change. The change description
continues.

FIXED: [ 123456 ] Bug title in case you fixed a bug
BUG: 123456

Discussion: https://invent.kde.org/utilities/krusader/-/merge_requests/7
```

Gitlab understands the discussion link and nicely [integrates it](https://invent.kde.org/help/user/markdown#special-gitlab-references) into the UI like this: utilities/krusader!7 ([see example commit](9198345c62ff6c2337fb37c9913bff933f57414b)).

Once you push the commit to the repository, it will be automatically discovered by the Gitlab and it will close the merge request.

If your changes are important enough to be included in the ChangeLog, please add a line to the commit message beginning with one of these keywords {`FIXED:`, `ADDED:`, `CHANGED:`, `REMOVED:`} and a description. For example:
```
CHANGED: When the big red button is pressed, foo is activated and not bar anymore
```
This line should not be the title of the commit message.
If your change spreads across multiple commits, use the keywords and discussion link in the merge commit message.
Keep in mind that the link and the keywords should go to either your single commit or branch merge commit, i.e. only appear once in history.
Check git history and ChangeLog for more examples of using the keywords.

If your change contains multiple commits, which you'd like to keep, and you are still on top of master, please use `--no-ff` to keep it as a branch merge:
```
git checkout master && git merge --no-ff feature/new-foobar-widget
```
and then amend the commit message accordingly. Substitute `master` with your base branch if needed.

### FAQ

> I want to fix a bug but there is no bug report for it yet. Do I need to file one?

No, not if you are certain that it really is a bug. Feel free to just submit a review.

> I am unsure about the right approach to solve an issue, what is the best place to discuss?

Ask us on the [developer mailing list](http://groups.google.com/group/krusader-devel).

> How do I start tracking Krusader bug reports?

1. Sign-in to [Bugzilla](https://bugs.kde.org/).
2. Go to Preferences → Email Preferences tab.
3. On the "User Watching" section add `krusader-bugs-null@kde.org` to your watch list.
