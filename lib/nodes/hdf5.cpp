/* A HDF5 node-type.
 *
 * This node-type reads the samples and writes them to a HDF5 file.
 *
 * Author: Alexandra Bach <alexandra.bach@eonerc.rwth-aachen.de>
 * SPDX-FileCopyrightText: 2014-2024 Institute for Automation of Complex Power Systems, RWTH Aachen University
 * SPDX-License-Identifier: Apache-2.0
 */

#include <iostream>
#include <string>
#include <sys/stat.h>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <H5Cpp.h>
#include <libgen.h>

#include <villas/exceptions.hpp>
#include <villas/node_compat.hpp>
#include <villas/nodes/hdf5.hpp>
#include <villas/sample.hpp>
#include <villas/super_node.hpp>
#include <villas/utils.hpp>

#define RANK        2

using namespace H5;
using namespace villas;
using namespace villas::node;
using namespace villas::utils;

HDF5node::HDF5node(const uuid_t &id, const std::string &name)
    : Node(id, name) {}

HDF5node::~HDF5node() {}

// int HDF5node::prepare() {
//   // state1 = setting1;

//   // if (setting2 == "double")
//   //   state1 *= 2;

//   // return 0;
//   // assert(state == State::PREPARED;);
//   state = State::PREPARED;
//   logger->debug("HDF5node::prepare()");
// 	return 0;
// }

int HDF5node::parse(json_t *json) {
  // default values
  location_name = "RWTH Aachen University";
  description_name = "This is a test dataset";
  project_name = "VILLASframework";
  author_name = "none";

  json_error_t err;
  // TODO: include uri where to store file
  int ret = json_unpack_ex(json, &err, 0, "{ s: s, s: s, s?: s, s?: s, s?: s, s?: s, s: s }",
              "file", &file_name, "dataset", &dataset_name, "location", &location_name,
              "description", &description_name, "project", &project_name, "author", &author_name,
              "unit", &unit);
  if (ret)
    throw ConfigError(json, err, "node-config-node-hdf5");

  logger->debug("HDF5node::parse() file_name: {}", file_name);

  return 0;
}

// int HDF5node::check() {
//   // if (setting1 > 100 || setting1 < 0)
//   //   return -1;

//   // if (setting2.empty() || setting2.size() > 10)
//   //   return -1;

//   return 0;
// }

int HDF5node::start() {
  // Create a new empty file which will be extended in _write

  hsize_t dims[2]    = {0, 0}; /* dataset dimensions at creation time */
  hsize_t maxdims[2] = {H5S_UNLIMITED, H5S_UNLIMITED};

  /* TODO: adapt chunk */
  hsize_t chunk_dims[2] = {2, 5};

  /* Create the data space with unlimited dimensions. */
  dataspace = H5Screate_simple(RANK, dims, maxdims);

  /* Create a new file. If file exists its contents will be overwritten. */
  file = H5Fcreate(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

  /* Modify dataset creation properties, i.e. enable chunking  */
  prop   = H5Pcreate(H5P_DATASET_CREATE);
  status = H5Pset_chunk(prop, RANK, chunk_dims);

  /* Create a new dataset within the file using chunk
      creation properties.  */
  dataset = H5Dcreate2(file, dataset_name, H5T_NATIVE_INT, dataspace, H5P_DEFAULT, prop, H5P_DEFAULT);

  logger->info("HDF5 file is initialized");

	return Node::start();
}

int HDF5node::stop()
{
	status = H5Dclose(dataset);
  status = H5Pclose(prop);
  status = H5Sclose(dataspace);
  status = H5Fclose(file);

	return Node::stop();
}

// int HDF5node::pause()
// {
// 	// TODO add implementation here
// 	return 0;
// }

// int HDF5node::resume()
// {
// 	// TODO add implementation here
// 	return 0;
// }

// int HDF5node::restart()
// {
// 	// TODO add implementation here
// 	return 0;
// }

// int HDF5node::reverse()
// {
// 	// TODO add implementation here
// 	return 0;
// }

// std::vector<int> HDF5node::getPollFDs()
// {
// 	// TODO add implementation here
// 	return {};
// }

// std::vector<int> HDF5node::getNetemFDs()
// {
// 	// TODO add implementation here
// 	return {};
// }

// struct villas::node::memory::Type * HDF5node::getMemoryType()
// {
//	// TODO add implementation here
// }

// const std::string &HDF5node::getDetails() {
//   details = fmt::format("setting1={}, setting2={}", setting1, setting2);
//   return details;
// }


// int HDF5node::_read(struct Sample *smps[], unsigned cnt) {

// }

// similar to file node-type: takes the samples and writes them to a HDF5 file
int HDF5node::_write(struct Sample *smps[], unsigned cnt) {

  // TODO: adjust
  // dimension of the data which should be written
  hsize_t dimsext[2]    = {4, 3}; /* extend dimensions */
  int     dataext[7][3] = {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}, {1, 2, 3}};

  // update dimension of row
  dim_row = dimsext[0] + dim_row;

  /* Extend the dataset. Dataset becomes 10 x 3  */
  hsize_t size[2];
  size[0] = dim_row;
  size[1] = dimsext[1];
  status  = H5Dset_extent(dataset, size);

  /* Select a hyperslab in extended portion of dataset  */
  filespace = H5Dget_space(dataset);
  hsize_t offset[2];
  offset[0] = dim_row;   // start at the end of the dataset
  offset[1] = 0;

  hsize_t dims[2];
  H5Sget_simple_extent_dims(filespace, dims, NULL);
  if (dim_row + dimsext[0] > dims[0])
    status    = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset, NULL, dimsext, NULL);
  else
    throw RuntimeError("HDF5node::_write() - dimension of the data which should be written is too large");
  /* Define memory space */
  memspace = H5Screate_simple(RANK, dimsext, NULL);

  /* Write the data to the extended portion of dataset  */
  status = H5Dwrite(dataset, H5T_NATIVE_INT, memspace, filespace, H5P_DEFAULT, dataext);

  status = H5Sclose(filespace);
  status = H5Sclose(memspace);

  logger->debug("HDF5node::_write() cnt: {}", cnt);

  return cnt;
}

// Register node
static char n[] = "hdf5";
static char d[] = "This node-type writes samples to hdf5 file format";
static NodePlugin<HDF5node, n, d,
                  (int)NodeFactory::Flags::SUPPORTS_READ |
                      (int)NodeFactory::Flags::SUPPORTS_WRITE |
                      (int)NodeFactory::Flags::SUPPORTS_POLL >
    p;
