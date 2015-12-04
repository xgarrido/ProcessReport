// \file falaise/snemo/reconstruction/process_report_module.cc

// Ourselves:
#include <snemo/reconstruction/process_report_module.h>

// Standard library:
#include <stdexcept>
#include <sstream>

// Third party:
// - Bayeux/datatools:
#include <datatools/service_manager.h>
// - Bayeux/geomtools:
#include <geomtools/geometry_service.h>
#include <geomtools/manager.h>
// - Bayeux/cuts:
#include <cuts/cut_service.h>
#include <cuts/cut_manager.h>

// This project (Falaise):
#include <falaise/snemo/processing/services.h>
#include <falaise/snemo/datamodels/data_model.h>

namespace snemo {

  namespace reconstruction {

    // Registration instantiation macro
    DPP_MODULE_REGISTRATION_IMPLEMENT(process_report_module,
                                      "snemo::reconstruction::process_report_module");

    const geomtools::manager & process_report_module::get_geometry_manager() const
    {
      return *_geometry_manager_;
    }

    void process_report_module::set_geometry_manager(const geomtools::manager & mgr_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _geometry_manager_ = &mgr_;
      return;
    }

    const cuts::cut_manager & process_report_module::get_cut_manager() const
    {
      return *_cut_manager_;
    }

    void process_report_module::set_cut_manager(const cuts::cut_manager & mgr_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is already initialized ! ");
      _cut_manager_ = &mgr_;
      return;
    }

    void process_report_module::_set_defaults()
    {
      _geometry_manager_  = 0;
      _cut_manager_  = 0;
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

      std::string geometry_label = snemo::processing::service_info::default_geometry_service_label();
      if (setup_.has_key("Geo_label")) {
        geometry_label = setup_.fetch_string("Geo_label");
      }
      // Geometry manager :
      if (_geometry_manager_ == 0) {
        DT_THROW_IF(geometry_label.empty(), std::logic_error,
                    "Module '" << get_name() << "' has no valid '" << "Geo_label" << "' property !");
        DT_THROW_IF(! service_manager_.has(geometry_label) ||
                    ! service_manager_.is_a<geomtools::geometry_service>(geometry_label),
                    std::logic_error,
                    "Module '" << get_name() << "' has no '" << geometry_label << "' service !");
        const geomtools::geometry_service & Geo
          = service_manager_.get<geomtools::geometry_service>(geometry_label);
        set_geometry_manager(Geo.get_geom_manager());
      }

      std::string cut_label = snemo::processing::service_info::default_cut_service_label();
      if (setup_.has_key("Cut_label")) {
        cut_label = setup_.fetch_string("Cut_label");
      }
      // Cut manager :
      if (_cut_manager_ == 0) {
        DT_THROW_IF(cut_label.empty(), std::logic_error,
                    "Module '" << get_name() << "' has no valid '" << "Cut_label" << "' property !");
        DT_THROW_IF(! service_manager_.has(cut_label) ||
                    ! service_manager_.is_a<cuts::cut_service>(cut_label),
                    std::logic_error,
                    "Module '" << get_name() << "' has no '" << cut_label << "' service !");
        const cuts::cut_service & Cut
          = service_manager_.get<cuts::cut_service>(cut_label);
        set_cut_manager(Cut.get_cut_manager());
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
    dpp::base_module::process_status process_report_module::process(datatools::things & data_record_)
    {
      DT_THROW_IF(! is_initialized(), std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      return dpp::base_module::PROCESS_SUCCESS;
    }

  } // end of namespace reconstruction

} // end of namespace snemo

/* OCD support */
#include <datatools/object_configuration_description.h>
DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::reconstruction::process_report_module, ocd_)
{
  ocd_.set_class_name("snemo::reconstruction::process_report_module");
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
DOCD_CLASS_SYSTEM_REGISTRATION(snemo::reconstruction::process_report_module,
                               "snemo::reconstruction::process_report_module")

// end of falaise/snemo/reconstruction/process_report_module.cc
