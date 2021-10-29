#!/usr/bin/env bash
set -euo pipefail
: "${2:?Version} ${1:?Package name}"
# e.g. fftw
name=${1}
# e.g. 3.3.10
ver=${2}
arch=${3:-big_sur}
outfile="${name}_${ver}-${arch}.tar.gz"
echo Downloading ${outfile}
digest=$(curl --disable --cookie /dev/null --globoff --show-error --header 'Accept-Language: en' --retry 3 --header 'Accept: application/vnd.oci.image.index.v1+json' --header 'Authorization: Bearer QQ==' --location --silent "https://ghcr.io/v2/homebrew/core/${name}/manifests/${ver}" | jq -r ".manifests[].annotations | select(.[\"org.opencontainers.image.ref.name\"] == \"${ver}.${arch}\") | .[\"sh.brew.bottle.digest\"]")
curl -o ${outfile} --disable --cookie /dev/null --globoff --show-error --header 'Accept-Language: en' --retry 3 --header 'Authorization: Bearer QQ==' --location --silent --request GET "https://ghcr.io/v2/homebrew/core/${name}/blobs/sha256:${digest}"
