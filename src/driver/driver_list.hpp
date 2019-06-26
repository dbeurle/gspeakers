/*
  speakerlist Copyright (C) 2002 Daniel Sundberg

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License version 2
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
*/

#ifndef __GFILTER_SPEAKERLIST
#define __GFILTER_SPEAKERLIST

#include "driver.hpp"

#include <iosfwd>
#include <string>
#include <vector>

/// This is a list to store all speakers in
class driver_list
{
public:
    driver_list() = default;

    /// Construct a part from an xml file
    driver_list(const std::string& filename);

    /// Convert data for a part to an xml node, throws std::runtime_error on failure
    void to_xml(const std::string& filename); // Maybe this one should throw an exception

    /// Print part data to stdout
    friend std::ostream& operator<<(std::ostream& output, const driver_list& driver_list);

    std::vector<driver>& data() noexcept { return m_speaker_list; }

    std::vector<driver> const& data() const noexcept { return m_speaker_list; }

    driver get_by_id_string(const std::string& id_string);

    /// Remove all items
    void clear();

protected:
    std::vector<driver> m_speaker_list;
};

#endif
