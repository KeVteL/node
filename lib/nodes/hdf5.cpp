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

// int HDF5node::start() {
//   assert(state == State::PREPARED ||
// 	       state == State::PAUSED);
//   logger->debug("HDF5node::start()");
// 	return Node::start();
// }

// int HDF5node::stop()
// {
// 	fclose(stream_in);
//   fclose(stream_out);

// 	return 0;
// }

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
  // Create a new file using the default property lists.
  H5File file(H5std_string(file_name), H5F_ACC_TRUNC);

  // Create the data space for the dataset. Defines the size and shape of the dataset.
  logger->info("smps[0]->length {}", smps[0]->length);
  // hsize_t dims[2] = {cnt, 1};     // cnt rowns, smps[0]->length columns
  hsize_t dims[2] = {4, 6};
  DataSpace dataspace(2, dims);

  // Create the dataset. Composes a collection of data elements, raw data and metadata.
  DataSet dataset = file.createDataSet(H5std_string(dataset_name), PredType::STD_I32BE, dataspace);

  // Write the data to the dataset.
  // for (unsigned j = 0; j < cnt; j++) {
  //   struct Sample *smp = smps[j];
  //   // ret = formatter->print(stream_out, smp, j+1);
  //   dataset.write(smp->data, PredType::NATIVE_DOUBLE);
  // }
  int data[4][6] = { {1, 2, 3, 4, 5, 6}, {7, 8, 9, 10, 11, 12}, {13, 14, 15, 16, 17, 18}, {19, 20, 21, 22, 23, 24}};
  dataset.write(data, H5::PredType::NATIVE_INT);

  // Create a dataspace for the attribute
  DataSpace attr_dataspace(H5S_SCALAR);

  // Create a string datatype
  StrType strdatatype(PredType::C_S1, H5T_VARIABLE); // for variable-length string

  // Create the timestamp attribute and write to it
  Attribute timestampAttribute = dataset.createAttribute("timestamp", strdatatype, attr_dataspace);

  // Get the current time
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  // Convert the current time to a string
  std::stringstream ss;
  ss << std::put_time(std::localtime(&now_c), "%Y-%m-%dT%H:%M:%S");
  std::string timestamp = ss.str();

  // Write the timestamp to the attribute
  timestampAttribute.write(strdatatype, timestamp);

  // Create the location attribute and write to it
  Attribute locationAttribute = dataset.createAttribute("location", strdatatype, attr_dataspace);
  // convert location_name to sting
  std::string location_name_string = location_name;
  locationAttribute.write(strdatatype, location_name_string);

  // Create the description attribute and write to it
  Attribute descriptionAttribute = dataset.createAttribute("description", strdatatype, attr_dataspace);
  std::string description_name_string = description_name;
  descriptionAttribute.write(strdatatype, description_name_string);

  // Create the project attribute and write to it
  Attribute projectAttribute = dataset.createAttribute("project", strdatatype, attr_dataspace);
  std::string project_name_string = project_name;
  projectAttribute.write(strdatatype, project_name_string);

  // Create the author attribute and write to it
  Attribute authorAttribute = dataset.createAttribute("author", strdatatype, attr_dataspace);
  std::string author_name_string = author_name;
  authorAttribute.write(strdatatype, author_name_string);

  logger->debug("attributes created and written");

  timestampAttribute.close();   // put in extra destroy function?
  locationAttribute.close();
  descriptionAttribute.close();
  projectAttribute.close();
  authorAttribute.close();
  dataset.close();
  file.close();

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
