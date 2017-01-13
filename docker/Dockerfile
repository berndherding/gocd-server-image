# providing a gocd server with private repo support.
# provide the ssh private key in /gocd/github.pem and
# make sure to have github grant access by uploading
# the ssh public key before.

FROM gocd/gocd-server:16.12.0

RUN  mkdir /gocd && \
     mkdir /gocd-plugins

COPY wait_for_go_server.sh /usr/bin
COPY known_hosts /etc/ssh/github_known_hosts
COPY ssh_config  /etc/ssh
COPY bh_init     /gocd/bh_init

# plugins

ADD  https://github.com/DennisDenuto/go-aws-ami-poller/releases/download/15.2.0/aws-ami-poller-15.2.0.jar /gocd-plugins
ADD  https://github.com/ind9/gocd-s3-artifacts/releases/download/v2.0.2/s3material-assembly-2.0.2.jar     /gocd-plugins
ADD  https://github.com/gocd-contrib/deb-repo-poller/releases/download/1.2/deb-repo-poller-1.2.jar        /gocd-plugins

RUN  chown -R go.go /gocd* /etc/ssh/github_known_hosts && \
     rm -fr /var/lib/go-server/plugins/external && \
     ln -sf /gocd-plugins /var/lib/go-server/plugins/external
RUN  chmod 0700 /gocd/bh_init

CMD  /gocd/bh_init
