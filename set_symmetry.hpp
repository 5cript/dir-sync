#pragma once

#include <thread>
#include <vector>

template <typename IteratorT>
class SymmetricDifferenceExtractor
{
public:
    using iterator = IteratorT;
    using container_type = std::vector <typename iterator::value_type>;
    using input_container_type = std::set <typename iterator::value_type>;

public:
    SymmetricDifferenceExtractor(
        input_container_type* lhsContainer,
        input_container_type* rhsContainer
    )
        : lhsBegin_{std::begin(*lhsContainer)}
        , rhsBegin_{std::begin(*rhsContainer)}
        , lhsProgress_{std::begin(*lhsContainer)}
        , rhsProgress_{std::begin(*rhsContainer)}
        , lhsSearcher_{std::begin(*lhsContainer)}
        , rhsSearcher_{std::begin(*rhsContainer)}
        , lhsEnd_{std::end(*lhsContainer)}
        , rhsEnd_{std::end(*rhsContainer)}
        , leftDiff_{}
        , rightDiff_{}
        , lhsContainer_{lhsContainer}
        , rhsContainer_{rhsContainer}
    {
    }

    /**
     *  Returns true if the difference has been extracted.
     */
    bool work(int count)
    {
        workLeftOnly(count);
        workRightOnly(count);

        return lhsProgress_ == lhsEnd_ && rhsProgress_ == rhsEnd_;
    }

    /**
     *  Returns true if the difference has been extracted.
     *  Left Difference = All Elements that are left, but not right.
     */
    bool workLeftOnly(int count)
    {
        for (int i = 0; i != count && lhsProgress_ != lhsEnd_; ++i)
            advanceSide(lhsProgress_, rhsSearcher_, leftDiff_, rhsBegin_, rhsEnd_, *lhsContainer_, *rhsContainer_);

        return lhsProgress_ == lhsEnd_;
    }

    /**
     *  Returns true if the difference has been extracted.
     *  Right Difference = All Elements that are right, but not left.
     */
    bool workRightOnly(int count)
    {
        for (int i = 0; i != count && rhsProgress_ != rhsEnd_; ++i)
            advanceSide(rhsProgress_, lhsSearcher_, rightDiff_, lhsBegin_, lhsEnd_, *rhsContainer_, *lhsContainer_);

        return rhsProgress_ == rhsEnd_;
    }

    /**
     *  Check whether all differences for right has been found.
     */
    bool leftWorkDone() const
    {
        return rhsProgress_ == rhsEnd_;
    }

    /**
     *  Check whether all differences for left has been found.
     */
    bool rightWorkDone() const
    {
        return lhsProgress_ == lhsEnd_;
    }

    bool isEmptyLeft() const
    {
        return leftDiff_.empty();
    }
    bool isEmptyRight() const
    {
        return rightDiff_.empty();
    }

    std::size_t leftSize() const
    {
        return leftDiff_.size();
    }
    std::size_t rightSize() const
    {
        return rightDiff_.size();
    }

    container_type* getLeftDifference()
    {
        return &leftDiff_;
    }
    container_type* getRightDifference()
    {
        return &rightDiff_;
    }
    container_type* getUnion()
    {
        return &union_;
    }
    container_type const& getLeftDifference() const
    {
        return leftDiff_;
    }
    container_type const& getRightDifference() const
    {
        return rightDiff_;
    }

private:
    void advanceSide(
        iterator& progressIterator,
        iterator& searchIterator,
        container_type& diff,
        iterator& begin,
        iterator& end,
        input_container_type& containerOfProgress,
        input_container_type& otherContainer
    )
    {
        if (begin == end)
        {
            diff.push_back(*progressIterator);
            ++progressIterator;
            return;
        }

        if (*progressIterator != *searchIterator)
            ++searchIterator;
        else
        {
            union_.push_back(*progressIterator);
            progressIterator = containerOfProgress.erase(progressIterator);
            otherContainer.erase(searchIterator);
            begin = std::begin(otherContainer);
            searchIterator = begin;
            return;
        }

        if (searchIterator == end)
        {
            diff.push_back(*progressIterator);
            progressIterator = containerOfProgress.erase(progressIterator);
            searchIterator = begin;
        }
    }

private:
    iterator lhsBegin_;
    iterator rhsBegin_;

    iterator lhsProgress_;
    iterator rhsProgress_;

    iterator lhsSearcher_;
    iterator rhsSearcher_;

    iterator lhsEnd_;
    iterator rhsEnd_;

    container_type leftDiff_; // elements that are left, but not right
    container_type rightDiff_; // elements that are right, but not left

    input_container_type* lhsContainer_;
    input_container_type* rhsContainer_;

    container_type union_;
};
