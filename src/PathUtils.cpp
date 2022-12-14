#include "PathUtils.h"
#include "SequenceUtils.h"


void InternalUtils::UPath::combinePurePath(
        const std::vector<std::vector<Path>>& purePaths,
        const Path& currPath,
        const std::vector<Sequence>& sequences,
        const Matrix& matrix, size_t maxLengthPath, int score,
        std::vector<bool>& outIsVisitedSequence, std::vector<std::pair<Path, int>>& outPathsAndScore)
{
    int currScore = score;

    for (size_t seqIdx = 0; seqIdx < purePaths.size(); ++seqIdx) {
        if (outIsVisitedSequence[seqIdx])
            continue;
        outIsVisitedSequence[seqIdx] = true;
        int nextScore = sequences[seqIdx].score();
        for (size_t pathIdx = 0; pathIdx < purePaths[seqIdx].size(); ++pathIdx) {
            std::vector<Path> resultPaths;
            if (isCreatedPathsAfterCurrent(currPath, purePaths[seqIdx][pathIdx], matrix, maxLengthPath, resultPaths))
                for (const Path& path: resultPaths) {
                    outPathsAndScore.push_back(std::pair(path, currScore + nextScore));
                    combinePurePath(
                                purePaths, path,
                                sequences, matrix,
                                maxLengthPath, currScore + nextScore,
                                outIsVisitedSequence, outPathsAndScore);
                }
        }
        outIsVisitedSequence[seqIdx] = false;
    }

}


bool InternalUtils::UPath::isCreatedPathsAfterCurrent(
        const Path& currPath,
        const Path& nextPath,
        const Matrix& matrix,
        const size_t maxLengthPath,
        std::vector<Path>& outPaths)
{
    if (hasIntersection(currPath, nextPath))
        return false;

    size_t overlapLength = 0;

    if (InternalUtils::UPath::hasOverlapping(currPath, nextPath, overlapLength)) {
        Path partOfNextPathForConcatenation({std::begin(nextPath.positions()) + overlapLength, std::end(nextPath.positions())});
        if (currPath.length() + partOfNextPathForConcatenation.length() <= maxLengthPath) {
            if (partOfNextPathForConcatenation.length() > 0) {
                if (currPath.positions()[currPath.positions().size() - 1].row() == partOfNextPathForConcatenation.positions()[0].row() ||
                        currPath.positions()[currPath.positions().size() - 1].column() == partOfNextPathForConcatenation.positions()[0].column()) {
                    outPaths.push_back(concatenatePaths(currPath, partOfNextPathForConcatenation));
                    return true;
                }
            }
            else {
                    outPaths.push_back(currPath);
                    return true;
                }
        }
    }

    return InternalUtils::USequence::isPossibleAddWastedMovesBetweenSequences(currPath, nextPath, matrix, maxLengthPath, outPaths);
}


bool InternalUtils::UPath::hasOverlapping(const Path& lhsPath, const Path& rhsPath, size_t& overlapLength)
{
    overlapLength = 0;
    auto beginIt = std::begin(rhsPath.positions());
    auto endIt = std::end(rhsPath.positions());

    while (endIt != beginIt) {

        if (endIt - beginIt == 1)
            if (*beginIt == *(--std::end(lhsPath.positions()))) {
                overlapLength = 1;
                return true;
            }

        auto it = std::search(std::begin(lhsPath.positions()), std::end(lhsPath.positions()), beginIt, endIt);
        if (it != std::end(lhsPath.positions()))
            if (endIt == std::end(rhsPath.positions()) || it + (endIt - beginIt) == std::end(lhsPath.positions())) {
                overlapLength = endIt - beginIt;
                return true;
            }

        --endIt;
    }
    return false;
}

bool InternalUtils::UPath::hasIntersection(const Path& lhsPath, const Path& rhsPath)
{
    std::vector<Position> lhs = lhsPath.positions();
    std::vector<Position> rhs = rhsPath.positions();
    std::sort(std::begin(lhs), std::end(lhs));
    std::sort(std::begin(rhs), std::end(rhs));
    std::vector<Position> intersection;
    std::set_intersection(std::begin(lhs), std::end(lhs), std::begin(rhs), std::end(rhs), std::back_inserter(intersection));

    if (intersection.empty())
        return false;

    size_t overlapLength;
    return !hasOverlapping(lhsPath, rhsPath, overlapLength);
}

Path InternalUtils::UPath::concatenatePaths(const Path& lhsPath, const Path& rhsPath)
{
    std::vector<Position> resultPosition(std::begin(lhsPath.positions()), std::end(lhsPath.positions()));
    resultPosition.insert(std::end(resultPosition), std::begin(rhsPath.positions()), std::end(rhsPath.positions()));
    return Path(resultPosition);
}

