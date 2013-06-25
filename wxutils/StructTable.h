#pragma once

namespace wxutils {

boost::optional<wxString> renderDate(const boost::posix_time::ptime & time);

class StructTableBase : public wxGridTableBase {
public:
protected:
    const wxString & one() const { return one_; }
    const wxString & zero() const { return zero_; }
    const wxString & renderBool(bool v) const { return v ? one() : zero(); }
private:
    static wxString one_;
    static wxString zero_;
};

template<class Item>
class StructTable : public StructTableBase {
public:
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
        wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, 0, value_.size());
        GetView()->ProcessTableMessage( msg );

        value_.clear();
    }

    void add(const Item & item)
    {
        value_.push_back(item);

        wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, 1);
        GetView()->ProcessTableMessage( msg );
    }

    void remove(size_t row)
    {
        if(row < value_.size())
        {
            value_.erase(value_.begin() + row);

            wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, row, 1);
            GetView()->ProcessTableMessage( msg );
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
        {
            wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_APPENDED, value_.size() - oldSize);
            GetView()->ProcessTableMessage(msg);
        } else if(value_.size() < oldSize)
        {
            wxGridTableMessage msg(this, wxGRIDTABLE_NOTIFY_ROWS_DELETED, value_.size(), oldSize - value_.size());
            GetView()->ProcessTableMessage(msg);
        }
    }

    int GetNumberRows()
    {
        return value_.size();
    }
protected:
    Value value_;
private:
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

}
