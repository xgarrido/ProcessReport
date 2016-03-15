/// \file falaise/snemo/processing/cut_report_driver.cc

// Ourselves:
#include <falaise/snemo/processing/cut_report_driver.h>

// Standard library:
#include <sstream>
#include <iomanip>
#include <regex>

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

      if (setup_.has_key("title")) {
        _title_ = setup_.fetch_string("title");
      }

      if (setup_.has_key("indent")) {
        _indent_ = setup_.fetch_string("indent");
      }

      if (setup_.has_key("print_report")) {
        const std::string value = setup_.fetch_string("print_report");
        if (value == "tree") {
          _print_report_ = PRINT_AS_TREE;
        } else if (value == "table") {
          _print_report_ = PRINT_AS_TABLE;
        } else if (value == "meter") {
          _print_report_ = PRINT_AS_METER;
        }
      }
      if (_print_report_ == PRINT_NONE) _print_report_ = PRINT_AS_METER;

      if (setup_.has_key("cuts")) {
        setup_.fetch("cuts", _cut_list_);
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
      _print_report_     = PRINT_NONE;
      _cut_list_.clear();
      _title_.clear();
      _indent_.clear();
      return;
    }

    // void cut_report_driver::process()
    // {
    //   DT_THROW_IF(! is_initialized(), std::logic_error, "Driver is not initialized !");
    //   return;
    // }

    void cut_report_driver::report(std::ostream & out_)
    {
      DT_THROW_IF(! has_cut_manager(), std::logic_error, "Missing cut manager !");
      if (! _title_.empty()) out_ << _title_ << std::endl;
      this->_report(out_);
      return;
    }

    void cut_report_driver::_report(std::ostream & out_)
    {
      const cuts::cut_manager & a_manager = get_cut_manager();
      if (_cut_list_.empty()) {
        const cuts::cut_handle_dict_type & a_cut_dict = a_manager.get_cuts();
        for (cuts::cut_handle_dict_type::const_iterator i = a_cut_dict.begin();
             i != a_cut_dict.end(); i++) {
          const std::string & a_cut_name = i->first;
          const cuts::cut_entry_type & a_cut_entry = i->second;
          if (! a_cut_entry.has_cut()) continue;
          _cut_list_.push_back(a_cut_name);
        }
      }

      for (cut_list_type::const_iterator icut = _cut_list_.begin();
           icut != _cut_list_.end(); ++icut) {
        const std::string & a_cut_name = *icut;

        auto is_separator = [] (const std::string & name_)
          {
            return std::regex_match(name_, std::regex("^-.*"));
          };

        // Check new serie of cut
        const bool start = icut == _cut_list_.begin() || is_separator(*std::prev(icut));

        // Do not treat separator as cut
        if (is_separator(a_cut_name)) continue;

        // No cut registered -> continue
        if (! a_manager.has(a_cut_name)) {
          DT_LOG_WARNING(get_logging_priority(), "No cut with name '" << a_cut_name << "' !");
          continue;
        }
        const cuts::i_cut & the_cut = a_manager.get(a_cut_name);

        // Cut statistics
        const size_t nae = the_cut.get_number_of_accepted_entries();
        const size_t nre = the_cut.get_number_of_rejected_entries();
        const size_t npe = the_cut.get_number_of_processed_entries();

        if (_print_report_ == PRINT_AS_TREE) {
          the_cut.tree_dump(out_, "Cut '" + a_cut_name + "'", _indent_);
        }
        if (_print_report_ == PRINT_AS_METER) {
          auto meter = [] (const size_t percent_)
            {
              const size_t sz = 10;
              const size_t idx = (percent_ == 0 ? 0 : percent_/sz+1);
              std::string a_meter;
              for (size_t i = 0; i < sz; i++) {
                if (i < idx) a_meter += "█";
                else         a_meter += " ";
              }
              return a_meter;
            };
          static size_t digit = 0;
          static size_t norm = 0;
          if (start) {
            digit = std::ceil(log10(npe+1));
            norm = npe;
            out_ << std::endl;
          }
          const double pae = (npe > 0 ? 100.0 * nae/norm : 0);
          const double pre = (npe > 0 ? 100.0 * nre/norm : 0);
          out_.setf(std::ios::fixed);
          out_.precision(1);
          out_ << _indent_ << "Cut '" << a_cut_name << "' statistics" << std::endl;
          out_ << _indent_ << " ↳ " << std::setw(digit)  << npe << " processed entries : "
               << meter(pae) << " " << std::setw(6) << pae << "% (" << std::right << std::setw(digit) << nae << ") "
               << meter(pre) << " " << std::setw(6) << pre << "% (" << std::right << std::setw(digit) << nre << ") "
               << std::endl;
        }
        if (_print_report_ == PRINT_AS_TABLE) {
          // Specific variable for table mode
          const size_t name_width = 25;
          const size_t nbr_width  = 8;
          static size_t column_width = 0;
          static std::ostringstream hline;
          if (hline.str().empty()) {
            std::ostringstream oss;
            oss << npe;
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
          }
          if (start) out_ << hline.str();
          out_.setf(std::ios::internal);
          if (a_cut_name.size() > name_width) {
            out_ << "| " << a_cut_name.substr(0, name_width) << "... | ";
          } else {
            out_ << "| " << a_cut_name << std::setfill(' ')
                 << std::setw(name_width - a_cut_name.size() + 6) << " | ";
          }
          out_.setf(std::ios::fixed);
          out_ << std::setw(column_width) << npe << " | "
               << std::setw(column_width) << nae << " | "
               << std::setw(nbr_width) << std::setprecision(2) << (npe > 0 ? 100.0 * nae/npe : 0) << "% | "
               << std::setw(column_width) << nre << " | "
               << std::setw(nbr_width) << std::setprecision(2) << (npe > 0 ? 100.0 * nre/npe : 0) << "% | "
               << std::endl;
          if (std::next(icut) == _cut_list_.end()) {
            out_ << hline.str() << std::endl;
          }
        }
      } // end of cut list
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
