A twin panel file manager. Written in C++11 with Qt5 and KF5.

= Useful links =

== General ==

* [[http://krusader.org/ | Project homepage]]

* [[http://download.kde.org/stable/krusader/ | Release downloads]]

* [[http://groups.google.com/group/krusader-users | User mailing-list]] (mainly for user questions)

* [[https://forum.kde.org/viewforum.php?f=225 | Forum]] (same purpose as user mailing list)

* [[https://bugs.kde.org/component-report.cgi?product=krusader | Bugzilla]] (for bug reports or wishes)

* [[https://www.kde.org/applications/utilities/krusader/ | Krusader introduction on kde.org]]

* [[https://store.kde.org/browse/cat/370/ord/top/ | Extensions]]

== Development ==

=== Krusader ===

* [[http://groups.google.com/group/krusader-devel | Developer mailing-list]] (for discussions, questions, etc.)

* Repositories
 * Source code
  * Read-only:  `git://anongit.kde.org/krusader`
  * Developer access: `git@git.kde.org:krusader.git`
  * [[https://cgit.kde.org/krusader.git | Web-based browser]]
  * [[https://github.com/KDE/krusader | GitHub mirror]] (read-only, no pull requests accepted)
 * Website
  * Read-only: `git://anongit.kde.org/websites/krusader-org.git`
  * Developer access: `git@git.kde.org:websites/krusader-org`
  * [[https://cgit.kde.org/websites/krusader-org.git | Web-based browser]]

* [[https://phabricator.kde.org/project/view/79/ | Phabricator project]] (reviews, outstanding tasks and more)

* [[https://build.kde.org/job/Extragear%20krusader%20kf5-qt5%20SUSEQt5.9/ | Jenkins]] (automatic build server, supports an RSS feed for the build status of `origin/master`)

* [[http://ebn.kde.org/krazy/reports/extragear/utils/krusader/index.html | Krazy]] (static code analysis service)

* [[https://techbase.kde.org/Policies/Frameworks_Coding_Style | Coding Style]] (KDE Frameworks coding style is preferred for Krusader project)

* [[ ./release-howto | Release Howto ]]

=== KDE ===

* KDE developer mailing-list: https://mail.kde.org/mailman/listinfo/kde-devel

* KDE system release list: https://mail.kde.org/mailman/listinfo/kde-announce

* KDE applications release list: https://mail.kde.org/mailman/listinfo/kde-announce-apps

* IRC-channel: chat.freenode.net/kde-devel (good place for quick questions regarding everything KDE related)

* Phabricator user guide, including how to submit patches: https://community.kde.org/Infrastructure/Phabricator
* Git hooks for Bugzilla: https://community.kde.org/Infrastructure/Git/Hooks#Keywords

* KIO API: https://api.kde.org/frameworks/kio/html/index.html . KIO is KDE's library for all file related operations (local and remote). Krusader does heavily depend on it.

* Porting notes, from KDE4 to KF5 (hopefully not important anymore): https://community.kde.org/Frameworks/Porting_Notes

==== Misc ====

* Recommended IDEs
** KDevelop: http://kdevelop.org
** Qt Creator: https://www.qt.io/ide/

* [[https://specifications.freedesktop.org/icon-naming-spec/icon-naming-spec-latest.html | Freedesktop icon names]] (official standard icon names, try to use these instead of names only included in Breeze to be more independent from the icon theme used)

* [[https://community.kde.org/Plasma/DeveloperGuide#Icon_Viewer:_Cuttlefish | Cuttlefish]] (a useful icon browser for KDE)

* Qt's new signal/slot syntax: https://wiki.qt.io/New_Signal_Slot_Syntax

= FAQs =

==== I want to contribute. How to submit my first patch? ====

Great!
# Get a [[https://community.kde.org/Infrastructure#Identity_Accounts | KDE identity account]]
# Create a new Diff:
 * https://phabricator.kde.org/differential/diff/create/ , with `krusader` as repository
 * or: (**preferred**) use `arc` (see https://community.kde.org/Infrastructure/Phabricator#Using_Arcanist)
# Wait for somebody to review and accept your diff.

==== My patch has been accepted, what now? ====

See [[https://community.kde.org/Infrastructure/Phabricator#Workflow | workflow documentation]], for your first patches you will usually have to ask your reviewers to land them.

==== I want to fix a bug but there is no bug report for it yet. Do I need to file one? ====

No, not if you are certain that it really is a bug. Feel free to just submit a review.

==== I am unsure about the right approach to solve an issue, what is the best place to discuss? ====

Ask us on the [[ http://groups.google.com/group/krusader-devel | developer mailing list ]].

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
