// Copyright (c) 2013-2014 Josh Blum
// SPDX-License-Identifier: BSL-1.0

#pragma once
#include <Pothos/Config.hpp>
#include <QObject>
#include <QString>
#include <QByteArray>
#include <QListWidget>
#include <vector>
#include <map>
#include <cassert>

/*!
 * StateManager is a templated class used for tracking state changes in the editor.
 */
template <typename StateType>
class StateManager
{
public:

    StateManager(void):
        _savedIndex(-1),
        _currentIndex(-1)
    {
        this->resetToDefault();
    }

    virtual ~StateManager(void)
    {
        return;
    }

    //! Reset to default state
    void resetToDefault(void)
    {
        _states.clear();
        _savedIndex = -1;
        _currentIndex = -1;
        this->change();
    }

    //! a change was made, the state is posted here
    void post(const StateType &state)
    {
        _states.resize(_currentIndex+1); //shrink to remove possible subsequent
        _states.push_back(state);
        _currentIndex = _states.size()-1;
        this->change();
    }

    //! how many states are in this manager
    size_t numStates(void) const
    {
        return _states.size();
    }

    //! Get the index of the current state
    size_t getCurrentIndex(void) const
    {
        return _currentIndex;
    }

    //! Get the index of the last saved state
    size_t getSavedIndex(void) const
    {
        return _savedIndex;
    }

    //! Get the state at the specified index
    const StateType &getStateAt(const size_t index) const
    {
        return _states.at(index);
    }

    //! get the current state
    const StateType &current(void) const
    {
        assert(_currentIndex >= 0);
        return _states.at(_currentIndex);
    }

    //! is there a subsequent state available?
    bool isSubsequentAvailable(void) const
    {
        return this->numStates() > _currentIndex+1;
    }

    //! is there a previous state available?
    bool isPreviousAvailable(void) const
    {
        return _currentIndex > 0;
    }

    //! reset the specified state index
    void resetTo(const int index)
    {
        assert(index >= 0);
        assert(index < _states.size());
        _currentIndex = index;
        this->change();
    }

    //! mark the current state as saved
    void saveCurrent(void)
    {
        _savedIndex = _currentIndex;
        this->change();
    }

    //! is the current state saved?
    bool isCurrentSaved(void) const
    {
        return _savedIndex == _currentIndex;
    }

    //! Called on change, can be overloaded
    virtual void change(void)
    {
        return;
    }

private:
    std::vector<StateType> _states;
    int _savedIndex;
    int _currentIndex;
};

/*!
 * GraphState is the actual state we store for a graph change.
 */
struct GraphState
{
    GraphState(void);
    GraphState(const QString &iconName, const QString &description, const QByteArray &dump = QByteArray());

    QString iconName;
    QString description;
    QByteArray dump;
};

class GraphStateManager : public QListWidget, public StateManager<GraphState>
{
    Q_OBJECT
public:
    GraphStateManager(QWidget *parent);

    ~GraphStateManager(void);

    void change(void);

signals:
    void newStateSelected(int);

private slots:
    void handleItemDoubleClicked(QListWidgetItem *);

private:
    std::map<QListWidgetItem *, int> _itemToIndex;
};
