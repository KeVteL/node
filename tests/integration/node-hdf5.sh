#!/bin/bash
#
# Test for the hdf5 node-type.
#
# Author: Alexandra Bach <alexandra.bach@eonerc.rwth-aachen.de>
# SPDX-FileCopyrightText: 2014-2024 Institute for Automation of Complex Power Systems, RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

set -e

DIR=$(mktemp -d)
pushd ${DIR}

function finish {
	popd
	rm -rf ${DIR}
}
trap finish EXIT

cat > config.json <<EOF
{
	"nodes": {
		"hdf5_node": {
			"type": "hdf5",
			"file": "test.h5",
			"dataset": "test,
			"unit": "none
		}
	},
	"siggen": {
		"type": "signal",
		"signal": "sine",
		"values": 1,
		"limit": 3,
		"rate": 10
	}
	"paths": [
		{
			"in": "siggen",
			"out": "hdf5_node",
		}
	]
}
EOF

villas node config.json
