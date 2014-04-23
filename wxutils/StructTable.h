/*
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
*/
#pragma once

namespace wxutils {

boost::optional<wxString> renderDate(const boost::posix_time::ptime & time);
const wxString & renderBool(bool v);

template<class I>
class StructTable : public wxGridTableBase {
public:
    typedef I Item;
    typedef std::vector<Item> Value;

    explicit StructTable(const Value & value)
        : value_(value)
    {
    }

    StructTable()
    {
    }

    template<class T>
    typename boost::enable_if<boost::is_same<T, int>, void>::type
    invertState(T row)
    {
        value_[row].active = !value_[row].active;
    }

    void clear()
    {
        tableMessage(wxGRIDTABLE_NOTIFY_ROWS_DELETED, 0, value_.size());

        value_.clear();
    }

    void add(const Item & item)
    {
        value_.push_back(item);

        tableMessage(wxGRIDTABLE_NOTIFY_ROWS_APPENDED, 1);
    }

    void remove(size_t row)
    {
        if(row < value_.size())
        {
            value_.erase(value_.begin() + row);

            tableMessage(wxGRIDTABLE_NOTIFY_ROWS_DELETED, row, 1);
        }
    }

    Value & value()
    {
        return value_;
    }

    const Value & value() const
    {
        return value_;
    }

    void updateNumberRows(size_t oldSize)
    {
        if(oldSize < value_.size())
            tableMessage(wxGRIDTABLE_NOTIFY_ROWS_APPENDED, value_.size() - oldSize);
        else if(value_.size() < oldSize)
            tableMessage(wxGRIDTABLE_NOTIFY_ROWS_DELETED, value_.size(), oldSize - value_.size());
    }

    const typename Value::value_type * at(size_t index) const
    {
        return index < value_.size() ? &value_[index] : nullptr;
    }

    int GetNumberRows()
    {
        return value_.size();
    }
protected:
    Value value_;
private:
    void tableMessage(int id, int i1, int i2 = -1)
    {
        auto view = GetView();
        if(view)
        {
            wxGridTableMessage msg(this, id, i1, i2);
            view->ProcessTableMessage(msg);
        }
    }
    
    bool IsEmptyCell(int row, int col)
    {
        return false;
    }

    wxString GetValue(int row, int col)
    {
        if(static_cast<size_t>(row) >= value_.size())
            return wxString();
        const Item & item = value_[row];
        return doGetValue(item, col);
    }

    void SetValue(int row, int col, const wxString & value)
    {
        if(static_cast<size_t>(row) >= value_.size())
            return;
        Item & item = value_[row];
        doSetValue(item, col, value);
    }

    wxString GetRowLabelValue(int col)
    {
        return wxString();
    }

    virtual wxString doGetValue(const Item & item, int col) = 0;
    virtual void doSetValue(Item & item, int col, const wxString & value) { BOOST_ASSERT(false); }
};

void refreshGridRect(wxGrid * grid, int row1, int col1, int row2, int col2);

}
