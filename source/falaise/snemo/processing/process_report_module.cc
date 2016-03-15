// \file falaise/snemo/processing/process_report_module.cc

// Ourselves:
#include <snemo/processing/process_report_module.h>

// Standard library:
#include <stdexcept>
#include <sstream>

// Third party:
// - Bayeux/datatools:
#include <bayeux/datatools/service_manager.h>
// - Bayeux/geomtools:
#include <bayeux/geomtools/geometry_service.h>
#include <bayeux/geomtools/manager.h>
// - Bayeux/cuts:
#include <bayeux/cuts/cut_service.h>
#include <bayeux/cuts/cut_manager.h>

// This project (Falaise):
#include <falaise/snemo/processing/services.h>
#include <falaise/snemo/processing/cut_report_driver.h>
#include <falaise/snemo/processing/geometry_report_driver.h>

namespace snemo {

  namespace processing {

    // Registration instantiation macro
    DPP_MODULE_REGISTRATION_IMPLEMENT(process_report_module,
                                      "snemo::processing::process_report_module")

    void process_report_module::_set_defaults()
    {
      _CRD_.reset();
      _GRD_.reset();
      _out_ = 0;
      return;
    }

    void process_report_module::initialize(const datatools::properties  & setup_,
                                           datatools::service_manager   & service_manager_,
                                           dpp::module_handle_dict_type & /* module_dict_ */)
    {
      DT_THROW_IF(is_initialized(),
                  std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");

      dpp::base_module::_common_initialize(setup_);

      DT_THROW_IF(! setup_.has_key("output"), std::logic_error,
                  "Missing 'output' key in module '" << get_name() <<  "'!");
      const std::string output_str = setup_.fetch_string("output");
      if (output_str == "clog") {
        _out_ = &std::clog;
      } else if (output_str == "cout") {
        _out_ = &std::cout;
      } else if (output_str == "file") {
        DT_THROW_IF(! setup_.has_key("output.filename"),
                    std::logic_error,
                    "Missing 'output.filename' property in module '"
                    << get_name () << "' ! ");
        const std::string _output_filename_ = setup_.fetch_string("output.filename");
      } else {
        DT_THROW_IF(true, std::logic_error,
                    "Invalid output label '" << output_str << " for module '" << get_name () << "' !");
      }
      DT_THROW_IF(_out_ == 0, std::logic_error, "Module '" << get_name() << "' has not output stream !");

      // Drivers :
      DT_THROW_IF(! setup_.has_key("drivers"), std::logic_error, "Missing 'drivers' key !");
      std::vector<std::string> driver_names;
      setup_.fetch("drivers", driver_names);
      for (std::vector<std::string>::const_iterator idriver = driver_names.begin();
           idriver != driver_names.end(); ++idriver) {
        const std::string & a_driver_name = *idriver;

        if (a_driver_name == snemo::processing::cut_report_driver::get_id()) {
          // Initialize Cut Report Driver
          _CRD_.reset(new snemo::processing::cut_report_driver);
          {
            std::string cut_label = snemo::processing::service_info::default_cut_service_label();
            if (setup_.has_key("Cut_label")) {
              cut_label = setup_.fetch_string("Cut_label");
            }
            // Cut manager :
            DT_THROW_IF(cut_label.empty(), std::logic_error,
                        "Module '" << get_name() << "' has no valid '" << "Cut_label" << "' property !");
            DT_THROW_IF(! service_manager_.has(cut_label) ||
                        ! service_manager_.is_a<cuts::cut_service>(cut_label),
                        std::logic_error,
                        "Module '" << get_name() << "' has no '" << cut_label << "' service !");
            const cuts::cut_service & Cut = service_manager_.get<cuts::cut_service>(cut_label);
            const cuts::cut_manager & the_cut_manager = Cut.get_cut_manager();
            _CRD_->set_cut_manager(the_cut_manager);
          }
          datatools::properties CRD_config;
          setup_.export_and_rename_starting_with(CRD_config, a_driver_name + ".", "");
          _CRD_->initialize(CRD_config);
        } else if (a_driver_name == snemo::processing::geometry_report_driver::get_id()) {
          // Initialize Geometry Report Driver
          _GRD_.reset(new snemo::processing::geometry_report_driver);
          {
            std::string geometry_label = snemo::processing::service_info::default_geometry_service_label();
            if (setup_.has_key("Geo_label")) {
              geometry_label = setup_.fetch_string("Geo_label");
            }
            // Geometry manager :
            DT_THROW_IF(geometry_label.empty(), std::logic_error,
                        "Module '" << get_name() << "' has no valid '" << "Geo_label" << "' property !");
            DT_THROW_IF(! service_manager_.has(geometry_label) ||
                        ! service_manager_.is_a<geomtools::geometry_service>(geometry_label),
                        std::logic_error,
                        "Module '" << get_name() << "' has no '" << geometry_label << "' service !");
            const geomtools::geometry_service & Geo
              = service_manager_.get<geomtools::geometry_service>(geometry_label);
            const geomtools::manager & the_geometry_manager = Geo.get_geom_manager();
            _GRD_->set_geometry_manager(the_geometry_manager);
          }
          datatools::properties GRD_config;
          setup_.export_and_rename_starting_with(GRD_config, a_driver_name + ".", "");
          _GRD_->initialize(GRD_config);
        } else {
          DT_THROW_IF(true, std::logic_error, "Driver '" << a_driver_name << "' does not exist !");
        }
      }

      // Tag the module as initialized :
      _set_initialized(true);
      return;
    }

    void process_report_module::reset()
    {
      DT_THROW_IF(! is_initialized(),
                  std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      if (_CRD_) _CRD_->report(*_out_);

      _set_initialized(false);
      _set_defaults();
      return;
    }

    // Constructor :
    process_report_module::process_report_module(datatools::logger::priority logging_priority_)
      : dpp::base_module(logging_priority_)
    {
      _set_defaults();
      return;
    }

    // Destructor :
    process_report_module::~process_report_module()
    {
      if (is_initialized()) process_report_module::reset();
      return;
    }

    // Processing :
    dpp::base_module::process_status process_report_module::process(datatools::things & /*data_record_*/)
    {
      DT_THROW_IF(! is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      return dpp::base_module::PROCESS_SUCCESS;
    }

  } // end of namespace processing

} // end of namespace snemo

/* OCD support */
#include <datatools/object_configuration_description.h>
  DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::processing::process_report_module, ocd_)
  {
    ocd_.set_class_name("snemo::processing::process_report_module");
    ocd_.set_class_description("A module that reports some information regarding the data processing");
    ocd_.set_class_library("Falaise_ProcessReport");

    // Invoke OCD support from parent class :
    dpp::base_module::common_ocd(ocd_);

    {
      // Description of the 'Geo_label' configuration property :
      datatools::configuration_property_description & cpd
        = ocd_.add_property_info();
      cpd.set_name_pattern("Geo_label")
        .set_terse_description("The label/name of the geometry service")
        .set_traits(datatools::TYPE_STRING)
        .set_mandatory(false)
        .set_long_description("This is the name of the service to be used as the \n"
                              "geometry service.                                 \n"
                              "This property is only used if no geometry manager \n"
                              "has been provided to the module.                   \n")
        .set_default_value_string(snemo::processing::service_info::default_geometry_service_label())
        .add_example("Use an alternative name for the geometry service:: \n"
                     "                                                   \n"
                     "  Geo_label : string = \"geometry2\"               \n"
                     "                                                   \n");
    }

    {
      // Description of the 'Cut_label' configuration property :
      datatools::configuration_property_description & cpd
        = ocd_.add_property_info();
      cpd.set_name_pattern("Cut_label")
        .set_terse_description("The label/name of the cut service")
        .set_traits(datatools::TYPE_STRING)
        .set_mandatory(false)
        .set_long_description("This is the name of the service to be used as the \n"
                              "cut service.                                      \n"
                              "This property is only used if no cut manager      \n"
                              "has been provided to the module.                  \n")
        .set_default_value_string(snemo::processing::service_info::default_cut_service_label())
        .add_example("Use an alternative name for the cut service:: \n"
                     "                                              \n"
                     "  Cut_label : string = \"cuts2\"              \n"
                     "                                              \n");
    }

    {
      configuration_property_description & cpd = ocd_.add_configuration_property_info();
      cpd.set_name_pattern("print_report")
        .set_terse_description("Flag to activate print report")
        .set_traits(datatools::TYPE_BOOLEAN)
        .set_mandatory(false)
        .set_default_value_boolean(false)
        .set_long_description("This flag activates the report")
        .add_example("Activate the printing of the report: :: \n"
                     "                                        \n"
                     "  print_report : boolean = true         \n"
                     "                                        \n"
                     )
        ;

    }

    // Additionnal configuration hints :
    ocd_.set_configuration_hints("Here is a full configuration example in the ``datatools::properties`` \n"
                                 "ASCII format::                                                        \n"
                                 "                                                                      \n"
                                 "  Geo_label : string = \"geometry\"                                   \n"
                                 "  Cut_label : string = \"cuts\"                                       \n"
                                 "                                                                      \n"
                                 );

    ocd_.set_validation_support(true);
    ocd_.lock();
    return;
  }
  DOCD_CLASS_IMPLEMENT_LOAD_END() // Closing macro for implementation
  DOCD_CLASS_SYSTEM_REGISTRATION(snemo::processing::process_report_module,
                                 "snemo::processing::process_report_module")

  // end of falaise/snemo/processing/process_report_module.cc
