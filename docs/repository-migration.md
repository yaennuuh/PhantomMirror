# Repository Migration

This project can be moved into a new dedicated GitHub repository without
preserving the existing Git history.

## Goal

- keep the current working tree
- start a fresh Git history from a single initial commit
- publish the code as source-available
- keep official releases and in-app updates tied only to the new repository

## Before you start

Make sure any uncommitted local work is either committed, stashed, or copied to
another safe location before replacing `.git`.

This repository currently has local modifications, so do not delete `.git`
until that work is accounted for.

## Create a new repository with fresh history

From the project root:

```powershell
Remove-Item -Recurse -Force .git
git init
git checkout -b main
git add .
git commit -m "Initial source-available release"
git remote add origin https://github.com/OWNER/REPO.git
git push -u origin main
```

If you want an extra safety net, copy the full folder first and perform the
steps in the copy instead of the current working directory.

## GitHub repository settings

Recommended repository setup:

- set the repository to Public if the source should be visible
- keep `LICENSE.md` as the source-available license document
- protect the `main` branch
- allow GitHub Actions to create and update releases

## Release flow

The release workflow supports two maintainable paths:

1. Push a semantic version tag such as `v0.1.0`
2. Run the `release-build` workflow manually in GitHub and enter `0.1.0`

Both paths build the Windows package, generate `SHA256SUMS.txt`, and create or
update the corresponding GitHub Release.

## First official release

After `main` is pushed to the new repository, use one of:

```powershell
git tag v0.1.0
git push origin v0.1.0
```

or trigger the workflow manually from the GitHub Actions UI with version
`0.1.0`.

The official workflow injects `github.repository` into
`PHANTOM_MIRROR_UPDATE_REPO`, so official builds always check the same
repository for updates. Local builds now default to no update repository unless
you configure one explicitly.
