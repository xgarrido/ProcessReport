/// \file falaise/snemo/processing/cut_report_driver.cc

// Ourselves:
#include <falaise/snemo/processing/cut_report_driver.h>

// Standard library:
#include <sstream>
#include <iomanip>

// Third party:
// - Bayeux/datatools:
#include <bayeux/datatools/properties.h>
// - Bayeux/cuts:
#include <bayeux/cuts/cut_manager.h>

namespace snemo {

  namespace processing {

    const std::string & cut_report_driver::get_id()
    {
      static const std::string s("CRD");
      return s;
    }

    void cut_report_driver::set_initialized(const bool initialized_)
    {
      _initialized_ = initialized_;
      return;
    }

    bool cut_report_driver::is_initialized() const
    {
      return _initialized_;
    }

    void cut_report_driver::set_logging_priority(const datatools::logger::priority priority_)
    {
      _logging_priority_ = priority_;
      return;
    }

    datatools::logger::priority cut_report_driver::get_logging_priority() const
    {
      return _logging_priority_;
    }

    void cut_report_driver::set_cut_manager(const cuts::cut_manager & mgr_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized !");
      _cut_manager_ = &mgr_;
      return;
    }

    const cuts::cut_manager & cut_report_driver::get_cut_manager() const
    {
      DT_THROW_IF(! has_cut_manager(), std::logic_error,
                  "No cut manager is setup !");
      return *_cut_manager_;
    }

    bool cut_report_driver::has_cut_manager() const
    {
      return _cut_manager_ != 0;
    }

    /// Constructor
    cut_report_driver::cut_report_driver()
    {
      _set_defaults();
      return;
    }

    /// Destructor
    cut_report_driver::~cut_report_driver()
    {
      if (is_initialized()) {
        reset();
      }
      return;
    }

    /// Initialize the driver through configuration properties
    void cut_report_driver::initialize(const datatools::properties & setup_)
    {
      DT_THROW_IF(is_initialized(), std::logic_error, "Driver is already initialized !");

      DT_THROW_IF(! has_cut_manager(), std::logic_error, "Missing cut manager !");
      DT_THROW_IF(! get_cut_manager().is_initialized(), std::logic_error,
                  "Cut manager is not initialized !");

      // Logging priority
      datatools::logger::priority lp = datatools::logger::extract_logging_configuration(setup_);
      DT_THROW_IF(lp == datatools::logger::PRIO_UNDEFINED,
                  std::logic_error,
                  "Invalid logging priority level for geometry manager !");
      set_logging_priority(lp);

      if (setup_.has_key("ordered_cuts")) {
        setup_.fetch("ordered_cuts", _ordered_cuts_);
      }

      set_initialized(true);
      return;
    }

    /// Reset the driver
    void cut_report_driver::reset()
    {
      DT_THROW_IF(! is_initialized(), std::logic_error, "Driver is not initialized !");

      _set_defaults();
      return;
    }

    void cut_report_driver::_set_defaults()
    {
      _initialized_      = false;
      _logging_priority_ = datatools::logger::PRIO_WARNING;
      _cut_manager_      = 0;
      return;
    }

    // void cut_report_driver::process()
    // {
    //   DT_THROW_IF(! is_initialized(), std::logic_error, "Driver is not initialized !");
    //   return;
    // }

    void cut_report_driver::report(std::ostream & out_) const
    {
      DT_THROW_IF(! has_cut_manager(), std::logic_error, "Missing cut manager !");
      this->_report(out_);
      return;
    }

    void cut_report_driver::_report(std::ostream & out_) const
    {
      const cuts::cut_manager & a_manager = get_cut_manager();

      auto print_cut = [] (const cuts::i_cut & cut_)
        {
        };

      const cuts::cut_handle_dict_type & a_cut_dict = a_manager.get_cuts();

      size_t column_width = 0;
      std::ostringstream hline;

      for (cuts::cut_handle_dict_type::const_iterator i = a_cut_dict.begin();
           i != a_cut_dict.end(); i++) {
        const std::string & the_cut_name = i->first;
        const cuts::cut_entry_type & the_cut_entry = i->second;
        if (! the_cut_entry.has_cut()) continue;
        const cuts::i_cut & the_cut = the_cut_entry.get_cut();
        // Specific variable for table mode
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
        if (std::next(i) == a_cut_dict.end()) {
          out_ << hline.str() << std::endl;
        }
      } // end of cut dictionary
      return;
    }

    // static
    void cut_report_driver::init_ocd(datatools::object_configuration_description & ocd_)
    {

      // Prefix "CRD" stands for "Cut Report Driver" :
      datatools::logger::declare_ocd_logging_configuration(ocd_, "fatal", "CRD.");

    }

  }  // end of namespace processing

}  // end of namespace snemo

/* OCD support */
#include <bayeux/datatools/object_configuration_description.h>
DOCD_CLASS_IMPLEMENT_LOAD_BEGIN(snemo::processing::cut_report_driver,ocd_)
{
  ocd_.set_class_name("snemo::processing::cut_report_driver");
  ocd_.set_class_description("A driver class to produce report related to cuts");
  ocd_.set_class_library("Falaise_ProcessReport");
  ocd_.set_class_documentation("This driver does a report of cuts efficiencies...\n");

  // Invoke specific OCD support :
  ::snemo::processing::cut_report_driver::init_ocd(ocd_);

  ocd_.set_validation_support(true);
  ocd_.lock();
  return;
}
DOCD_CLASS_IMPLEMENT_LOAD_END() // Closing macro for implementation
DOCD_CLASS_SYSTEM_REGISTRATION(snemo::processing::cut_report_driver,
                               "snemo::processing::cut_report_driver")

// end of falaise/snemo/processing/cut_report_driver.cc
