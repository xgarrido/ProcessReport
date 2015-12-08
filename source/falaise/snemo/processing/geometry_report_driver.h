/// \file falaise/snemo/processing/geometry_report_driver.h
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
 *   A driver class that produce a report related to detector geometry.
 *
 * History:
 *
 */

#ifndef FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_GEOMETRY_REPORT_DRIVER_H
#define FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_GEOMETRY_REPORT_DRIVER_H 1

// Third party:
// - Bayeux/datatools
#include <bayeux/datatools/logger.h>
#include <bayeux/datatools/bit_mask.h>

namespace datatools {
  class properties;
}

namespace geomtools {
  class manager;
}

namespace snemo {

  namespace processing {

    /// \brief Geometry report driver
    class geometry_report_driver
    {
    public:

      /// Report format type
      enum report_format_type {
        PRINT_NONE     = 0,
        PRINT_AS_TREE  = datatools::bit_mask::bit00,
        PRINT_AS_TABLE = datatools::bit_mask::bit01,
        PRINT_IN_FILE  = datatools::bit_mask::bit02
      };

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

      /// Check the geometry manager
      bool has_geometry_manager() const;

      /// Address the geometry manager
      void set_geometry_manager(const geomtools::manager & mgr_);

      /// Return a non-mutable reference to the geometry manager
      const geomtools::manager & get_geometry_manager() const;

      /// Constructor:
      geometry_report_driver();

      /// Destructor:
      ~geometry_report_driver();

      /// Initialize the driver through configuration properties
      void initialize(const datatools::properties & setup_);

      /// Reset the driver
      void reset();

      /// Main driver method
      void process();

      /// OCD support:
      static void init_ocd(datatools::object_configuration_description & ocd_);

    protected:

      /// Set default values to class members:
      void _set_defaults();

    private:

      /// Measure particle charge:
      void _print_geometry_report_() const;

    private:

      bool _initialized_;                             //<! Initialize flag
      datatools::logger::priority _logging_priority_; //<! Logging flag
      const geomtools::manager * _geometry_manager_;        //!< The geometry manager
      uint32_t _print_report_;                        //!< Print report format
    };

  }  // end of namespace processing

}  // end of namespace snemo

#include <bayeux/datatools/ocd_macros.h>

// Declare the OCD interface of the module
DOCD_CLASS_DECLARATION(snemo::processing::geometry_report_driver)

#endif // FALAISE_PROCESSREPORT_PLUGIN_PROCESSING_GEOMETRY_REPORT_DRIVER_H

// end of falaise/snemo/processing/geometry_report_driver.h
/*
** Local Variables: --
** mode: c++ --
** c-file-style: "gnu" --
** tab-width: 2 --
** End: --
*/
