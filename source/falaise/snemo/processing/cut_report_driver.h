/// \file falaise/snemo/processing/cut_report_driver.h
/* Author(s)     : Xavier Garrido <garrido@lal.in2p3.fr>
 * Creation date : 2015-12-08
 * Last modified : 2015-12-08
 *
 * Copyright (C) 2015 Xavier Garrido <garrido@lal.in2p3.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Description:
 *
 *   A driver class that produce a report related to cuts.
 *
 * History:
 *
 */

#ifndef FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_CUT_REPORT_DRIVER_H
#define FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_CUT_REPORT_DRIVER_H 1

// Third party:
// - Bayeux/datatools
#include <bayeux/datatools/logger.h>
#include <bayeux/datatools/bit_mask.h>
// - Bayeux/cuts
#include <bayeux/cuts/cut_tools.h>

namespace datatools {
  class properties;
}

namespace cuts {
  class cut_manager;
}

namespace snemo {

  namespace processing {

    /// \brief Cut report driver
    class cut_report_driver
    {
    public:

      /// Return driver id
      static const std::string & get_id();

      /// Setting initialization flag
      void set_initialized(const bool initialized_);

      /// Getting initialization flag
      bool is_initialized() const;

      /// Setting logging priority
      void set_logging_priority(const datatools::logger::priority priority_);

      /// Getting logging priority
      datatools::logger::priority get_logging_priority() const;

      /// Check the cut manager
      bool has_cut_manager() const;

      /// Address the cut manager
      void set_cut_manager(const cuts::cut_manager & mgr_);

      /// Return a non-mutable reference to the cut manager
      const cuts::cut_manager & get_cut_manager() const;

      /// Constructor:
      cut_report_driver();

      /// Destructor:
      ~cut_report_driver();

      /// Initialize the driver through configuration properties
      void initialize(const datatools::properties & setup_);

      /// Reset the driver
      void reset();

      /// Main report method
      void report(std::ostream & out_) const;

      /// OCD support:
      static void init_ocd(datatools::object_configuration_description & ocd_);

    protected:

      /// Set default values to class members:
      void _set_defaults();

      ///
      void _report(std::ostream & out_) const;

    private:

      bool _initialized_;                             //<! Initialize flag
      datatools::logger::priority _logging_priority_; //<! Logging flag
      const cuts::cut_manager * _cut_manager_;        //!< The cut manager
      cuts::ordered_cut_list_type _ordered_cuts_;     //!< Ordered list of cuts
    };

  }  // end of namespace processing

}  // end of namespace snemo

#include <bayeux/datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::processing::cut_report_driver)

#endif // FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_CUT_REPORT_DRIVER_H

// end of falaise/snemo/processing/cut_report_driver.h
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
