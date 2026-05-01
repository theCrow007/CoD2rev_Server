FROM debian:bookworm-slim AS build

RUN apt-get update \
	&& apt-get install -y --no-install-recommends build-essential ca-certificates make \
	&& rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY Makefile ./
COPY src ./src

RUN make ARCH=x64

FROM debian:bookworm-slim

RUN apt-get update \
	&& apt-get install -y --no-install-recommends libstdc++6 \
	&& rm -rf /var/lib/apt/lists/* \
	&& useradd --create-home --uid 10001 cod2

WORKDIR /cod2
RUN mkdir -p /cod2/bin /game /home/cod2/.callofduty2 \
	&& chown -R cod2:cod2 /cod2 /home/cod2/.callofduty2

COPY --from=build /src/bin/cod2rev_lnxded_x64 /cod2/bin/cod2rev_lnxded_x64

USER cod2
EXPOSE 28962/udp

ENV COD2_PARAMS="+set fs_cdpath /game +set fs_homepath /home/cod2/.callofduty2 +set dedicated 2 +set net_port 28962 +set developer 1 +set sv_maxclients 32 +set g_gametype tdm +set sv_cheats 1 +set sv_cracked 1 +devmap mp_toujane"

ENTRYPOINT ["/bin/sh", "-lc", "exec /cod2/bin/cod2rev_lnxded_x64 $COD2_PARAMS"]
