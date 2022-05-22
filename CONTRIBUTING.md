# Contributing to Simdsp

IMPORTANT: Simdsp has fun requirements around what C++ features it can use where.  If "we must avoid the standard
library in contexts where the compiler may choose to use a function from a different translation unit" and "we must be
careful not to violate the one definition rule" are confusing statements, you probably want to ask before you do
anything.  I want to document these better later but right now this is just a prototype/derisking project (if this
paragraph is still here and this has made it into Synthizer feel free to ping me and I will remove it...).

Simdsp is open to pull requests for bug fixes and new features.  Please create issues discussing new features before
starting work on them, in case they need to wait, shouldn't be done, or will be more complicated than you might think.

## Licensing

This project is under the [unlicense](./LICENSE).  Please include the folllowing at the bottom of your PR comment:

```
I have read CONTRIBUTING.md, am the sole contributor to this pull request, and license my contributions under the Unlicense.
```

If there's multiple contributors to your PR, let me know in the PR. I'll need them to all have GitHub accounts, and to
leave a comment to the effect of them agreeing that their contribution is under the unlicense as well.

We might make this more formal later, but the unlicense is liberal enough to even allow relicensing, so for the time
being there's no CLA.
