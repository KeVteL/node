/* A HDF5 node-type.
 *
 * This node-type reads the samples and writes them to a HDF5 file.
 *
 * Author: Alexandra Bach <alexandra.bach@eonerc.rwth-aachen.de>
 * SPDX-FileCopyrightText: 2014-2024 Institute for Automation of Complex Power Systems, RWTH Aachen University
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <H5Cpp.h>
#include <H5File.h>

#include <villas/format.hpp>
#include <villas/node.hpp>
#include <villas/node/config.hpp>
#include <villas/timing.hpp>

using namespace H5;

namespace villas {
namespace node {

// Forward declarations
class NodeCompat;
struct Sample;

class HDF5node : public Node {

protected:
  Format *formatter;
  FILE *stream_in;        // File to read from.
  FILE *stream_out;       // File to write to.

  char *uri_tmpl; // Format string for file name.
  char *uri;      // Real file name.

  int flush;              // Flush / upload file contents after each write.
  size_t buffer_size_out; // Defines size of output stream buffer. No buffer is created if value is set to zero.
  size_t buffer_size_in;  // Defines size of input stream buffer. No buffer is created if value is set to zero.

  // names
  const char *file_name;
  const char *dataset_name;
  const char *dataspace_name;
  const char *location_name;
  const char *description_name;
  const char *project_name;
  const char *author_name;
  const char *unit;

  // HDF5
  // H5File file(H5std_string(uri), H5F_ACC_TRUNC);
  // DataSet *dataset;
  // DataSpace *dataspace;
  // Attribute *timestamp_attr;
  // Attribute *location_attr;
  // Attribute *description_attr;
  // Attribute *project_attr;
  // Attribute *author_attr;

  struct attributes {
    int *timestamp;
    char *location;
    char *description;
    char *project;
    char *author;
  } attributes;

  // virtual int _read(struct Sample *smps[], unsigned cnt);

  virtual int _write(struct Sample *smps[], unsigned cnt);

public:
  HDF5node(const uuid_t &id = {}, const std::string &name = "");

  /* All of the following virtual-declared functions are optional.
	 * Have a look at node.hpp/node.cpp for the default behaviour.
	 */

  virtual ~HDF5node();

  // virtual int prepare();

  virtual int parse(json_t *json);

  // Validate node configuration
  // virtual int check();

  // virtual int start();

  // virtual int stop();

  // virtual
  // int pause();

  // virtual
  // int resume();

  // virtual
  // int restart();

  // virtual
  // int reverse();

  // virtual
  // std::vector<int> getPollFDs();

  // virtual
  // std::vector<int> getNetemFDs();

  // virtual
  // struct villas::node::memory::Type * getMemoryType();

  // virtual const std::string &getDetails();
};

} // namespace node
} // namespace villas
