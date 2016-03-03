/// \file falaise/snemo/processing/geometry_report_driver.cc

// Ourselves:
#include <falaise/snemo/processing/geometry_report_driver.h>

// Standard library:
#include <sstream>
#include <iomanip>

// Third party:
// - Bayeux/datatools:
#include <bayeux/datatools/properties.h>
// - Bayeux/cuts:
#include <bayeux/geomtools/manager.h>

namespace snemo {

  namespace processing {

    const std::string & geometry_report_driver::get_id()
    {
      static const std::string s("GRD");
      return s;
    }

    void geometry_report_driver::set_initialized(const bool initialized_)
    {
      _initialized_ = initialized_;
      return;
    }

    bool geometry_report_driver::is_initialized() const
    {
      return _initialized_;
    }

    void geometry_report_driver::set_logging_priority(const datatools::logger::priority priority_)
    {
      _logging_priority_ = priority_;
      return;
    }

    datatools::logger::priority geometry_report_driver::get_logging_priority() const
    {
      return _logging_priority_;
    }

    void geometry_report_driver::set_geometry_manager(const geomtools::manager & mgr_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized !");
      _geometry_manager_ = &mgr_;
      return;
    }

    const geomtools::manager & geometry_report_driver::get_geometry_manager() const
    {
      DT_THROW_IF(! has_geometry_manager(), std::logic_error,
                  "No cut manager is setup !");
      return *_geometry_manager_;
    }

    bool geometry_report_driver::has_geometry_manager() const
    {
      return _geometry_manager_ != 0;
    }

    /// Constructor
    geometry_report_driver::geometry_report_driver()
    {
      _set_defaults();
      return;
    }

    /// Destructor
    geometry_report_driver::~geometry_report_driver()
    {
      if (is_initialized()) {
        reset();
      }
      return;
    }

    /// Initialize the driver through configuration properties
    void geometry_report_driver::initialize(const datatools::properties & setup_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized !");

      DT_THROW_IF(! has_geometry_manager(), std::logic_error, "Missing geometry manager !");
      DT_THROW_IF(! get_geometry_manager().is_initialized(), std::logic_error,
                  "Geometry manager is not initialized !");

      // Logging priority
      datatools::logger::priority lp = datatools::logger::extract_logging_configuration(setup_);
      DT_THROW_IF(lp == datatools::logger::PRIO_UNDEFINED,
                  std::logic_error,
                  "Invalid logging priority level for geometry manager !");
      set_logging_priority(lp);

      if (setup_.has_key("print_report")) {
        const std::string value = setup_.fetch_string("print_report");
        if (value == "tree") {
          _print_report_ |= PRINT_AS_TREE;
        } else if (value == "table") {
          _print_report_ |= PRINT_AS_TABLE;
        } else if (value == "file") {
          DT_LOG_WARNING(get_logging_priority(), "Saving cut report within file is not yet supported !");
          // _print_report_ |= PRINT_IN_FILE;
          // DT_THROW_IF(! a_config.has_key("print_report.filename"), std::logic_error, "Missing cut report file name !");
          // _print_report_filename_ = a_config.fetch_string("print_report.filename");
        } else {
          DT_THROW_IF(true, std::logic_error, "Unkown format type '" << value << "' !");
        }
      }

      set_initialized(true);
      return;
    }

    /// Reset the driver
    void geometry_report_driver::reset()
    {
      DT_THROW_IF(! is_initialized(), std::logic_error, "Driver is not initialized !");

      geometry_report_driver::_print_geometry_report_();

      _set_defaults();
      return;
    }

    void geometry_report_driver::_set_defaults()
    {
      _initialized_      = false;
      _logging_priority_ = datatools::logger::PRIO_WARNING;
      _print_report_     = PRINT_NONE;
      _geometry_manager_ = 0;
      return;
    }

    void geometry_report_driver::process()
    {
      DT_THROW_IF(! is_initialized(), std::logic_error, "Driver is not initialized !");
      return;
    }


    void geometry_report_driver::_print_geometry_report_() const
    {
      DT_THROW_IF(! has_geometry_manager(), std::logic_error, "Missing cut manager !");
      // const geomtools::manager & a_manager = get_geometry_manager();
      return;
    }

    // static
    void geometry_report_driver::init_ocd(datatools::object_configuration_description & ocd_)
    {

      // Prefix "GRD" stands for "Geometry Report Driver" :
      datatools::logger::declare_ocd_logging_configuration(ocd_, "fatal", "GRD.");

    }

  }  // end of namespace processing

}  // end of namespace snemo

/* OCD support */
#include <bayeux/datatools/object_configuration_description.h>
DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::processing::geometry_report_driver,ocd_)
{
  ocd_.set_class_name("snemo::processing::geometry_report_driver");
  ocd_.set_class_description("A driver class to produce report related to geometry");
  ocd_.set_class_library("Falaise_ProcessReport");
  ocd_.set_class_documentation("This driver does a report of geometry settings.\n");

  // Invoke specific OCD support :
  ::snemo::processing::geometry_report_driver::init_ocd(ocd_);

  ocd_.set_validation_support(true);
  ocd_.lock();
  return;
}
DOCD_CLASS_IMPLEMENT_LOAD_END() // Closing macro for implementation
DOCD_CLASS_SYSTEM_REGISTRATION(snemo::processing::geometry_report_driver,
                               "snemo::processing::geometry_report_driver")

// end of falaise/snemo/processing/geometry_report_driver.cc
