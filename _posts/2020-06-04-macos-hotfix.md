---
layout: post
title: BrainFlow 3.0.3
subtitle: Hotfix for MacOS
tags: [release]
---

It seems like in one of dependencies we use for MacOS cert expired or smth else happend. As a result on MacOS this dynamic library could not be loaded anymore.

We signed this library by our own cert instead.

If you are a user and see this error:

```
"OSError: dlopen(/Users/hyemin/opt/anaconda3/envs/notebook/lib/python3.7/site-packages/brainflow/lib/libneurosdk-shared.dylib, 6): no suitable image found.  Did find: /Users/hyemin/opt/anaconda3/envs/notebook/lib/python3.7/site-packages/brainflow/lib/libneurosdk-shared.dylib: code signature invalid for '/Users/hyemin/opt/anaconda3/envs/notebook/lib/python3.7/site-packages/brainflow/lib/libneurosdk-shared.dylib'"
```

Move to new version and it will solve your issue.

To prevent such issues in future we will get rid of linking this library and load it manually in runtime instead.

[Commit with fix](https://github.com/brainflow-dev/brainflow/commit/735c9d66c2015a109fd70aae55e9fac9e59656e0)
