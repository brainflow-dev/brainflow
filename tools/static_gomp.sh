#!/bin/bash

find /opt -name libgomp.spec -exec bash -c "echo *link_gomp: -l:libgomp.a %{static: -ldl } > {}" \;
find /usr/lib -name libgomp.spec -exec bash -c "echo *link_gomp: -l:libgomp.a %{static: -ldl } > {}" \;