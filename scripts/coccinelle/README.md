This directory contains a collection of coccinelle [1] scripts used to
update OpenNetworkLinux for API changes in newer kernel versions.

Semantic patches were applied via

  spatch --sp-file <patch>.cocci --in-place --dir packages/

then converted into commits/patches.

[1] https://coccinelle.gitlabpages.inria.fr/website/
