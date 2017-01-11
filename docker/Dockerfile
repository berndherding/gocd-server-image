# providing a gocd server with private repo support.
# provide the ssh private key in /gocd/github.pem and
# make sure to have github grant access by uploading
# the ssh public key before.

FROM gocd/gocd-server:16.12.0

COPY known_hosts /etc/ssh/github_known_hosts
COPY ssh_config  /etc/ssh

COPY wait_for_go_server.sh /usr/bin

RUN  chown go.go /etc/ssh/github_known_hosts

RUN  mkdir /gocd
RUN  chown go.go /gocd

COPY bh_init     /gocd/bh_init
RUN  chown go.go /gocd/bh_init
RUN  chmod 0700  /gocd/bh_init

# plugins

RUN  mkdir /gocd-plugins
ADD  https://github.com/DennisDenuto/go-aws-ami-poller/releases/download/15.2.0/aws-ami-poller-15.2.0.jar /gocd-plugins
ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3material-assembly-2.0.2.jar     /gocd-plugins
ADD  https://github.com/gocd-contrib/deb-repo-poller/releases/download/1.2/deb-repo-poller-1.2.jar        /gocd-plugins

CMD /gocd/bh_init
