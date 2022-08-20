# MIT License

# Copyright (c) 2018-2020 by Maxim Biro <nurupo.contributions@gmail.com>

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
# Install verifying the hash

# Verifying PGP signature on CI is error-prone, keyservers often fail to return
# the key and even if they do, `gpg --verify` returns success with a revoked
# or expired key. Thus it's probably better to verify the signature yourself,
# on your local machine, and then rely on the hash on the CI.

# Set the variables below for the version of ci_release_publisher you would like
# to use. The set values are provided as an example and are likely very out of
# date.

#VERSION="0.1.0rc1"
#FILENAME="ci_release_publisher-$VERSION-py3-none-any.whl"
#HASH="5a7f0ad6ccfb6017974db42fb1ecfe8b3f9cc1c16ac68107a94979252baa16e3"

# Get Python >=3.5
if [ "$TRAVIS_OS_NAME" == "osx" ]; then
  brew update

  # Upgrade Python 2 to Python 3
  brew upgrade python || true

  # Print python versions
  python --version || true
  python3 --version || true
  pyenv versions || true

  cd .
  cd "$(mktemp -d)"
  virtualenv env -p python3
  set +u
  source env/bin/activate
  set -u
  cd -

  # make sha256sum available
  export PATH="/usr/local/opt/coreutils/libexec/gnubin:$PATH"
elif [ "$TRAVIS_OS_NAME" == "linux" ]; then
  # Print python versions
  python --version || true
  python3 --version || true
  pyenv versions || true

  # Install Python >=3.5 that has a non-zero patch version
  # (we assume the zero patch versions to be potentially buggier than desired)
  pyenv global $(pyenv versions | grep -o ' 3\.[5-99]\.[1-99]' | tail -n1)
fi

pip install --upgrade pip

check_sha256()
{
  if ! ( echo "$1  $2" | sha256sum -c --status - ); then
    echo "Error: sha256 of $2 doesn't match the known one."
    echo "Expected: $1  $2"
    echo -n "Got: "
    sha256sum "$2"
    exit 1
  else
    echo "sha256 matches the expected one: $1"
  fi
}

# Don't install again if already installed.
# OSX keeps re-installing it tough, as it uses a temp per-script virtualenv.
if ! pip list --format=columns | grep '^ci-release-publisher '; then
  cd .
  cd "$(mktemp -d)"
  VERSION="0.2.0"
  FILENAME="ci_release_publisher-$VERSION-py3-none-any.whl"
  HASH="da7f139e90c57fb64ed2eb83c883ad6434d7c0598c843f7be7b572377bed4bc4"
  pip download ci_release_publisher==$VERSION
  check_sha256 "$HASH" "$FILENAME"
  pip install --no-index --find-links "$PWD" "$FILENAME"
  cd -
fi
