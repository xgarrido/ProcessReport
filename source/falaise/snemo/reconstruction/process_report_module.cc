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
      _print_report_ = PRINT_NONE;
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

      // Tag the module as initialized :
      _set_initialized(true);
      return;
    }

    void process_report_module::reset()
    {
      DT_THROW_IF(! is_initialized(),
                  std::logic_error,
                  "Module '" << get_name() << "' is not initialized !");

      process_report_module::_print_cut_report();

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

    void process_report_module::_print_cut_report(std::ostream & out_) const
    {
      DT_THROW_IF(_cut_manager_ == 0, std::logic_error, "Missing cut manager !");
      // out_ << "List of created cuts (by order of appeareance) :" << std::endl;
      const cuts::cut_manager & a_manager = get_cut_manager();
      const cuts::cut_handle_dict_type & a_cut_dict = a_manager.get_cuts();
      // Specific variable for table mode
      size_t column_width = 0;
      std::ostringstream hline;
      for (cuts::cut_handle_dict_type::const_iterator i = a_cut_dict.begin();
           i != a_cut_dict.end(); i++) {
        const std::string & the_cut_name = i->first;
        const cuts::cut_entry_type & the_cut_entry = i->second;
        if (! the_cut_entry.has_cut()) continue;
        const cuts::i_cut & the_cut = the_cut_entry.get_cut();
        if (_print_report_ & PRINT_AS_TREE) {
          std::ostringstream indent_oss;
          if (boost::next(i) == a_cut_dict.end()) {
            out_ << datatools::i_tree_dumpable::last_tag;
            indent_oss << datatools::i_tree_dumpable::last_skip_tag;
          } else {
            out_ << datatools::i_tree_dumpable::tag;
            indent_oss << datatools::i_tree_dumpable::skip_tag;
          }
          out_ << "Cut '" << the_cut_name << "' status report : " << std::endl;
          the_cut.tree_dump(out_, "", indent_oss.str());
        } else if (_print_report_ & PRINT_AS_TABLE) {
          const size_t name_width = 25;
          const size_t nbr_width  = 8;
          if (hline.str().empty()) {
            std::ostringstream oss;
            oss << the_cut.get_number_of_processed_entries();
            column_width = oss.str().size();
            // Header line
            hline << "+" << std::setfill('-') << std::setw(name_width + 6)
                  << "+" << std::setfill('-') << std::setw(column_width + 3)
                  << "+" << std::setfill('-') << std::setw(column_width + 3)
                  << "+" << std::setfill('-') << std::setw(nbr_width + 4)
                  << "+" << std::setfill('-') << std::setw(column_width + 3)
                  << "+" << std::setfill('-') << std::setw(nbr_width + 4)
                  << "+" << std::endl;
            out_ << hline.str();
            out_ << "| " << "Cut name" << std::setw(name_width - 2)
                 << "| " << std::setw(column_width + 3)
                 << "| " << "Accepted" << std::setw(column_width + nbr_width - 1)
                 << "| " << "Rejected" << std::setw(column_width + nbr_width - 2)
                 << "|" << std::endl;
            out_ << hline.str();
          }
          out_.setf(std::ios::internal);
          if (the_cut_name.size() > name_width) {
            out_ << "| " << the_cut_name.substr(0, name_width) << "... | ";
          } else {
            out_ << "| " << the_cut_name << std::setfill(' ')
                 << std::setw(name_width - the_cut_name.size() + 6) << " | ";
          }
          const size_t nae = the_cut.get_number_of_accepted_entries();
          const size_t nre = the_cut.get_number_of_rejected_entries();
          const size_t npe = the_cut.get_number_of_processed_entries();
          out_.setf(std::ios::fixed);
          out_ << std::setw(column_width) << npe << " | "
               << std::setw(column_width) << nae << " | "
               << std::setw(nbr_width) << std::setprecision(2) << (npe > 0 ? 100.0 * nae/npe : 0) << "% | "
               << std::setw(column_width) << nre << " | "
               << std::setw(nbr_width) << std::setprecision(2) << (npe > 0 ? 100.0 * nre/npe : 0) << "% | "
               << std::endl;
          if (boost::next(i) == a_cut_dict.end()) {
            out_ << hline.str() << std::endl;
          }
        }
      } // end of cut dictionary
      return;
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
  DOCD_CLASS_SYSTEM_REGISTRATION(snemo::reconstruction::process_report_module,
                                 "snemo::reconstruction::process_report_module")

  // end of falaise/snemo/reconstruction/process_report_module.cc
