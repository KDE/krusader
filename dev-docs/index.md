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
* [Automatic builds](https://build.kde.org/job/Extragear/job/krusader/job/stable-kf5-qt5%20SUSEQt5.14/)
* [Krazy: static code analysis](http://ebn.kde.org/krazy/reports/extragear/utils/krusader/index.html)
* [Krusader coding style](https://techbase.kde.org/Policies/Frameworks_Coding_Style) — same style as for Frameworks
* [Release Howto](release-howto.md) — step-by-step release guide
* [krusader.org website repository](https://invent.kde.org/websites/krusader-org) — the website is deployed via CI from this repo

### General KDE development resources

* [KDE developer mailing-list](https://mail.kde.org/mailman/listinfo/kde-devel)
* [KDE general announcements mailing-list](https://mail.kde.org/mailman/listinfo/kde-announce)
* [KDE applications release announcements mailing-list](https://mail.kde.org/mailman/listinfo/kde-announce-apps) — [we announce](release-howto.md#send-a-letter-to-mailing-lists) Krusader releases here
* IRC-channel: chat.freenode.net/kde-devel — for quick questions regarding anything related to KDE development
* [Git hooks for KDE Bugzilla](https://community.kde.org/Infrastructure/Git/Hooks#Keywords) — use these keywords in your commits

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


```
===== End of updated part of the page =====
```


= FAQs =

==== I want to contribute. How to submit my first patch? ====

Great!
# Get a [KDE identity account](https://community.kde.org/Infrastructure#Identity_Accounts)
# Create a new Diff:
 * https://phabricator.kde.org/differential/diff/create/ , with `krusader` as repository
 * or: (**preferred**) use `arc` (see https://community.kde.org/Infrastructure/Phabricator#Using_Arcanist)
# Wait for somebody to review and accept your diff.

==== My patch has been accepted, what now? ====

See [workflow documentation](https://community.kde.org/Infrastructure/Phabricator#Workflow), for your first patches you will usually have to ask your reviewers to land them.

==== I want to fix a bug but there is no bug report for it yet. Do I need to file one? ====

No, not if you are certain that it really is a bug. Feel free to just submit a review.

==== I am unsure about the right approach to solve an issue, what is the best place to discuss? ====

Ask us on the [developer mailing list](http://groups.google.com/group/krusader-devel).

==== I want to know when a Krusader bug report is created (or changed) ====

To know when a Krusader bug report is created (or when an existing bug report is changed):
* You must have an account in bugs.kde.org, then you can go to «//https://bugs.kde.org > Preferences > Email preferences//» and on the "User Watching" section you can add: krusader-bugs-null@kde.org

= Commit/Patch guidelines =

Familiarize yourself with commit message style by reading several commit messages in the repository. Notice that for simple changes we use single commits and for multiple related changes we use branches and merge them into master with a descriptive merge commit message.

If your commit/patch fixes a bug reported on bugs.kde.org, use both the `FIXED:` keyword with the bug number in square brackets and bug title, and 'BUG:' keyword that triggers the Git hook to close the bug. Refer to an example below.

Your change must be reviewed and approved before you can push it to master branch. We currently use Phabricator platform for code reviews.

Your commit (or branch merge if your change is a series of commits) must contain the code review link. It's recommended to place it on the last line and separate it from summary with an empty line. For example:

> One-line commit title
>
> Description of the change. The change description
> continues.
>
> FIXED: [ 123456 ] Bug title in case you fixed a bug
> BUG: 123456
>
> Differential Revision: https://phabricator.kde.org/D12345

Once you push the commit to the repository, it will be automatically discovered by the Phabricator and it will close your code review (aka Differential) and put references to your commits.

If your changes are important enough to be included in the ChangeLog, please add a line to the commit message (summary text for the Differential on Phabricator for patches) beginning with one of these keywords {`FIXED:`, `ADDED:`, `CHANGED:`, `REMOVED:`} and a description. For example:
> CHANGED: When the big red button is pressed, foo is activated and not bar anymore
This line should not be the title of the commit message/differential. Check git history and ChangeLog for more examples of using the keywords.

= Best practice "Git vs. Phabricator" workflow =

NOTE: this is only a guideline. You can use your own workflow but be sure it is "good". If you are new to Phabricator (or Git), better do as it is described here. If you are new to Krusader development and have suggestions on how to modify it, please contact krusader-devel group.

Phabricator has a major design flaw: it alters Git commits. When a differential diff is created with `arc diff`, the last commit message is modified with potentially a huge unformatted summary text and information specific to Phabricator. These are definitely NOT good commit messages. Second, when landing the diff, all commits are squashed into one. This is against the "split changes into logical chunks"-rule for version control systems. This is default and can be changed (with the `--merge` flag). It is still annoying.

Here is are workflow for circumventing unwanted modifications:
* Create a new feature branch:
`git checkout -b feature/new-foobar-widget`
* Make your changes/commits normally you would in Git
* When done, you can upload your changes to Phabricator for review but **before** copy your feature branch for arc:
`git checkout -b feature/new-foobar-widget-arc`
* Now upload with nice information:
`arc diff --reviewers "#krusader"`

> Added new foobar widget
>
> Summary:
> Bla, Bla, Bla Mr. Freeman.
> ...

NOTE: If your base branch is not `master`, specify the base branch as well. For example,
`arc diff --reviewers "#krusader" stable`.

* In case you need to make updates based on reviewer comment or further testing, commit changes to the arc branch and run `arc diff <base-branch>`. Revision will be updated with your changes. Cherry-pick your commits to non-arc branch.

* Wait until your changes are accepted. Merge the non-arc branch with master (or simply cherry-pick and amend your commit in case of a single commit).
`git checkout master && git merge feature/new-foobar-widget`
Make sure you add code review link and keywords according to commit guidelines. The link and the keywords should go to either your single commit or branch merge commit, i.e. only appear once in history.

If your change contains multiple commits and you are still on top of master, please use `--no-ff` to keep it as a branch merge:
`git checkout master && git merge --no-ff feature/new-foobar-widget`

NOTE: Substitute `master` with your base branch if needed.

Example of branch merge commit:
> Added new foobar widget
>
> Merge branch 'feature/new-foobar-widget'
>
> Differential Revision: https://phabricator.kde.org/D1234

Done with `git pull`! The Differential diff should be automatically be closed (because of the reference).
