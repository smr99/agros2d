// This file is part of Agros2D.
//
// Agros2D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros2D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros2D.  If not, see <http://www.gnu.org/licenses/>.
//
// hp-FEM group (http://hpfem.org/)
// University of Nevada, Reno (UNR) and University of West Bohemia, Pilsen
// Email: agros2d@googlegroups.com, home page: http://hpfem.org/agros2d/

#include "problem.h"

#include "scene.h"
#include "scenemarker.h"
#include "scenesolution.h"
#include "module.h"
#include "module_agros.h"
#include "coupling.h"
#include "solver.h"
#include "progressdialog.h"


Field::Field(FieldInfo *fieldInfo) : m_fieldInfo(fieldInfo)
{

}

bool Field::solveInitVariables()
{
    //TODO transient
    //
    //
    //    // transient
    //    if (Util::scene()->problemInfo()->analysisType() == AnalysisType_Transient)
    //    {
    //        if (!Util::scene()->problemInfo()->timeStep.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->timeTotal.evaluate()) return false;
    //        if (!Util::scene()->problemInfo()->initialCondition.evaluate()) return false;
    //    }

    if(!Util::scene()->boundaries->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    if(!Util::scene()->materials->filter(m_fieldInfo).evaluateAllVariables())
        return false;

    return true;
}

Block::Block(QList<FieldInfo *> fieldInfos, QList<CouplingInfo*> couplings, ProgressItemSolve* progressItemSolve, Problem* parent) :
    m_progressItemSolve(progressItemSolve), m_couplings(couplings), m_parentProblem(parent)
{
    foreach (FieldInfo* fi, fieldInfos)
    {
        Field* field = new Field(fi);
        foreach (CouplingInfo* couplingInfo, Util::scene()->couplingInfos())
        {
            if(couplingInfo->isWeak() && (couplingInfo->targetField() == fi))
            {
                field->m_couplingSources.push_back(couplingInfo);
            }
        }

        m_fields.append(field);
    }

}


void Block::solve()
{
    cout << "############ Block::Solve() ################" << endl;

    Solver<double> solver;

    foreach (Field* field, m_fields)
    {
        if(! field->solveInitVariables())
            assert(0); //TODO co to znamena?
    }

    m_wf = new WeakFormAgros<double>(this);

    solver.init(m_progressItemSolve, m_wf, this);

    if(this->isTransient()){
        solver.solveInitialTimeStep();
        for(int i = 0; i < 40; i++)
            solver.solveTimeStep(this->timeStep());
    }
    else
        solver.solveSimple();
}

bool Block::isTransient() const
{
    foreach (Field *field, m_fields)
    {
        if(field->fieldInfo()->analysisType() == AnalysisType_Transient)
            return true;
    }

    return false;
}

int Block::numSolutions() const
{
    int num = 0;

    foreach (Field *field, m_fields)
    {
        num += field->fieldInfo()->module()->number_of_solution();
    }

    return num;
}

int Block::offset(Field *fieldParam) const
{
    int offset = 0;

    foreach (Field* field, m_fields)
    {
        if(field == fieldParam)
            return offset;
        else
            offset += field->fieldInfo()->module()->number_of_solution();
    }

    assert(0);
}

LinearityType Block::linearityType() const
{
    int linear = 0, newton = 0;
    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->linearityType == LinearityType_Linear)
            linear++;
        if(fieldInfo->linearityType == LinearityType_Newton)
            newton++;
    }
    assert(linear * newton == 0); // all hard coupled fields has to be solved by the same method

    if(linear)
        return LinearityType_Linear;
    else if(newton)
        return  LinearityType_Newton;
    else
        assert(0);
}

double Block::nonlinearTolerance() const
{
    double tolerance = 10e20;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearTolerance < tolerance)
            tolerance = fieldInfo->nonlinearTolerance;
    }

    return tolerance;
}

int Block::nonlinearSteps() const
{
    int steps = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->nonlinearSteps > steps)
            steps = fieldInfo->nonlinearSteps;
    }

    return steps;
}

double Block::timeStep() const
{
    double step = 0;

    foreach (Field* field, m_fields)
    {
        FieldInfo* fieldInfo = field->fieldInfo();
        if(fieldInfo->analysisType() == AnalysisType_Transient)
        {
            if (step == 0)
                step = fieldInfo->timeStep().number();

            //TODO zatim moc nevim
            assert(step == fieldInfo->timeStep().number());
        }
    }

    return step;
}

bool Block::contains(FieldInfo *fieldInfo) const
{
    foreach(Field* field, m_fields)
    {
        if(field->fieldInfo() == fieldInfo)
            return true;
    }
    return false;
}

Field* Block::field(FieldInfo *fieldInfo) const
{
    foreach (Field* field, m_fields)
    {
        if(fieldInfo == field->fieldInfo())
            return field;
    }

    return NULL;
}

ostream& operator<<(ostream& output, const Block& id)
{
    output << "Block ";
    return output;
}


Problem::Problem()
{
    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;

    m_meshInitial = NULL;
}

Problem::~Problem()
{
    clear();
}

void Problem::clear()
{
    Util::solutionStore()->clearAll();

    if (m_meshInitial)
        delete m_meshInitial;
    m_meshInitial = NULL;


    m_timeStep = 0;
    m_isSolved = false;
    m_isSolving = false;
}

const bool REVERSE_ORDER_IN_BLOCK_DEBUG_REMOVE = false;

void Problem::createStructure()
{
    foreach(Block* block, m_blocks)
    {
        delete block;
    }
    m_blocks.clear();

    Util::scene()->synchronizeCouplings();

    //copy lists, items will be removed from them
    QList<FieldInfo *> fieldInfos = Util::scene()->fieldInfos().values();
    QList<CouplingInfo* > couplingInfos = Util::scene()->couplingInfos().values();

    while (!fieldInfos.empty()){
        QList<FieldInfo*> blockFieldInfos;
        QList<CouplingInfo*> blockCouplingInfos;

        //first find one field, that is not weakly coupled and dependent on other fields
        bool dependent;
        foreach (FieldInfo* fi, fieldInfos)
        {
            dependent = false;

            foreach (CouplingInfo* ci, couplingInfos)
            {
                if(ci->isWeak() && (ci->targetField() == fi) && fieldInfos.contains(ci->sourceField()))
                    dependent = true;
            }

            // this field is not weakly dependent, we can put it into this block
            if(! dependent){
                blockFieldInfos.push_back(fi);
                fieldInfos.removeOne(fi);
                break;
            }
        }
        assert(! dependent);

        // find hardly coupled fields to construct block
        bool added = true;
        while(added)
        {
            added = false;

            // first check whether there is related coupling
            foreach (CouplingInfo* checkedCouplingInfo, couplingInfos)
            {
                foreach (FieldInfo* checkedFieldInfo, blockFieldInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this coupling is related, add it to the block
                        added = true;
                        blockCouplingInfos.push_back(checkedCouplingInfo);
                        couplingInfos.removeOne(checkedCouplingInfo);
                    }
                }
            }

            // check for fields related to allready included couplings
            foreach (FieldInfo* checkedFieldInfo, fieldInfos)
            {
                foreach (CouplingInfo* checkedCouplingInfo, blockCouplingInfos)
                {
                    if(checkedCouplingInfo->isHard() && checkedCouplingInfo->isRelated(checkedFieldInfo))
                    {
                        //this field is related (by this coupling)
                        added = true;

                        //TODO for debugging only
                        if(REVERSE_ORDER_IN_BLOCK_DEBUG_REMOVE)
                            blockFieldInfos.push_front(checkedFieldInfo);
                        else
                            blockFieldInfos.push_back(checkedFieldInfo);

                        fieldInfos.removeOne(checkedFieldInfo);
                    }
                }
            }
        }

        // now all hard-coupled fields are here, create block
        m_blocks.append(new Block(blockFieldInfos, blockCouplingInfos, m_progressItemSolve, this));
    }

}

void Problem::mesh()
{
    ProgressItemMesh* pim = new ProgressItemMesh();
    pim->mesh();
}

void Problem::solve(SolverMode solverMode)
{
    logMessage("SceneSolution::solve()");

    cout << "####***##### PROBLEM::Solve() #####******#####" << endl;

    setVerbose(true);

    if (isSolving()) return;

    clear();
    m_isSolving = true;

    // clear problem
    //clear(solverMode == SolverMode_Mesh || solverMode == SolverMode_MeshAndSolve);

    // open indicator progress
    Indicator::openProgress();

    // save problem
    ErrorResult result = Util::scene()->writeToFile(tempProblemFileName() + ".a2d");
    if (result.isError())
        result.showDialog();

    createStructure();

    Util::scene()->setActiveViewField(Util::scene()->fieldInfos().values().at(0));

    mesh();
    emit meshed();

    Util::scene()->clearSolutions();
    Util::solutionStore()->clearAll();

    assert(isMeshed());

    if (solverMode == SolverMode_MeshAndSolve)
    {
        foreach (Block* block, m_blocks)
        {
            block->solve();
        }
    }

    Util::scene()->setActiveTimeStep(Util::solutionStore()->lastTimeStep(Util::scene()->activeViewField()));

    // delete temp file
    if (Util::scene()->problemInfo()->fileName == tempProblemFileName() + ".a2d")
    {
        QFile::remove(Util::scene()->problemInfo()->fileName);
        Util::scene()->problemInfo()->fileName = "";
    }

    // close indicator progress
    Indicator::closeProgress();

    m_isSolving = false;
    if (solverMode == SolverMode_MeshAndSolve)
    {
        m_isSolved = true;
        emit solved();
        //TODO emit timeStepChanged(false);
    }
}

ProgressDialog* Problem::progressDialog()
{
    return m_progressDialog;
}

//*************************************************************************************************

const int notFoundSoFar = -999;

void SolutionStore::clearAll()
{
    m_multiSolutions.clear();
}

void SolutionStore::clearOne(FieldSolutionID solutionID)
{
    m_multiSolutions.remove(solutionID);
}

SolutionArray<double> SolutionStore::solution(FieldSolutionID solutionID, int component)
{
    return multiSolution(solutionID).component(component);
}

MultiSolutionArray<double> SolutionStore::multiSolution(FieldSolutionID solutionID)
{
    if(m_multiSolutions.contains(solutionID))
        return m_multiSolutions[solutionID];

    return MultiSolutionArray<double>();
}

bool SolutionStore::contains(FieldSolutionID solutionID)
{
    return m_multiSolutions.contains(solutionID);
}

MultiSolutionArray<double> SolutionStore::multiSolution(BlockSolutionID solutionID)
{
    MultiSolutionArray<double> msa;
    foreach(Field *field, solutionID.group->m_fields)
    {
        msa.append(multiSolution(solutionID.fieldSolutionID(field->fieldInfo())));
    }

    return msa;
}

void SolutionStore::saveSolution(FieldSolutionID solutionID,  MultiSolutionArray<double> multiSolution)
{
    cout << "$$$$$$$$  Saving solution " << solutionID << ", now solutions: " << m_multiSolutions.size() << ", time " << multiSolution.component(0).time << endl;
    assert(!m_multiSolutions.contains(solutionID));
    assert(solutionID.timeStep >= 0);
    assert(solutionID.adaptivityStep >= 0);
    m_multiSolutions[solutionID] = multiSolution;
}

void SolutionStore::saveSolution(BlockSolutionID solutionID, MultiSolutionArray<double> multiSolution)
{
    foreach(Field* field, solutionID.group->m_fields)
    {
        FieldSolutionID fieldSID = solutionID.fieldSolutionID(field->fieldInfo());
        MultiSolutionArray<double> fieldMultiSolution = multiSolution.fieldPart(solutionID.group, field->fieldInfo());
        saveSolution(fieldSID, fieldMultiSolution);
    }
}

int SolutionStore::lastTimeStep(FieldInfo *fieldInfo)
{
    int timeStep = notFoundSoFar;
    foreach(FieldSolutionID sid, m_multiSolutions.keys())
    {
        if((sid.group == fieldInfo) && (sid.timeStep > timeStep))
            timeStep = sid.timeStep;
    }

    return timeStep;
}

int SolutionStore::lastTimeStep(Block *block)
{
    int timeStep = lastTimeStep(block->m_fields.at(0)->fieldInfo());

    foreach(Field* field, block->m_fields)
    {
        assert(lastTimeStep(field->fieldInfo()) == timeStep);
    }

    return timeStep;
}

double SolutionStore::lastTime(FieldInfo *fieldInfo)
{
    int timeStep = lastTimeStep(fieldInfo);
    double time = notFoundSoFar;

    foreach(FieldSolutionID id, m_multiSolutions.keys())
    {
        if((id.group == fieldInfo) && (id.timeStep == timeStep) && (id.exists()))
        {
            if(time == notFoundSoFar)
                time = m_multiSolutions[id].component(0).time;
            else
                assert(time == m_multiSolutions[id].component(0).time);
        }
    }
    assert(time != notFoundSoFar);
    return time;
}

double SolutionStore::lastTime(Block *block)
{
    double time = lastTime(block->m_fields.at(0)->fieldInfo());

    foreach(Field* field, block->m_fields)
    {
        assert(lastTime(field->fieldInfo()) == time);
    }

    return time;

}

int SolutionStore::lastAdaptiveStep(FieldInfo *fieldInfo, int timeStep)
{
    if(timeStep == -1)
        timeStep = lastTimeStep(fieldInfo);

    int adaptiveStep = notFoundSoFar;
    foreach(FieldSolutionID sid, m_multiSolutions.keys())
    {
        if((sid.group == fieldInfo) && (sid.timeStep == timeStep) && (sid.adaptivityStep > adaptiveStep))
            adaptiveStep = sid.adaptivityStep;
    }

    return adaptiveStep;
}

int SolutionStore::lastAdaptiveStep(Block *block, int timeStep)
{
    int adaptiveStep = lastAdaptiveStep(block->m_fields.at(0)->fieldInfo());

    foreach(Field* field, block->m_fields)
    {
        assert(lastAdaptiveStep(field->fieldInfo()) == adaptiveStep);
    }

    return adaptiveStep;
}

FieldSolutionID SolutionStore::lastTimeAndAdaptiveSolution(FieldInfo *fieldInfo, SolutionType solutionType)
{
    FieldSolutionID solutionID;
    solutionID.group = fieldInfo;
    solutionID.adaptivityStep = lastAdaptiveStep(fieldInfo);
    solutionID.timeStep = lastTimeStep(fieldInfo);
    solutionID.solutionType = solutionType;

    //cout << solutionID << endl;

    if(solutionType == SolutionType_Finer)
    {
        solutionID.solutionType = SolutionType_Reference;
        if(! m_multiSolutions.contains(solutionID))
        {
            solutionID.solutionType = SolutionType_Normal;
        }
    }

    assert(m_multiSolutions.contains(solutionID));
    if(solutionType == SolutionType_Reference)
    {
        FieldSolutionID solutionIDNormal = solutionID;
        solutionIDNormal.solutionType = SolutionType_Normal;
        assert(m_multiSolutions.contains(solutionIDNormal));
    }

    return solutionID;
}

BlockSolutionID SolutionStore::lastTimeAndAdaptiveSolution(Block *block, SolutionType solutionType)
{
    FieldSolutionID fsid = lastTimeAndAdaptiveSolution(block->m_fields.at(0)->fieldInfo(), solutionType);
    BlockSolutionID bsid = fsid.blockSolutionID(block);


    foreach(Field* field, block->m_fields)
    {
        assert(fsid == lastTimeAndAdaptiveSolution(field->fieldInfo(), solutionType));
    }

    return bsid;
}

QList<double> SolutionStore::timeLevels(FieldInfo *fieldInfo)
{
    QList<double> list;

    foreach(FieldSolutionID fsid, m_multiSolutions.keys())
    {
        if(fsid.group == fieldInfo)
        {
            double time = m_multiSolutions[fsid].component(0).time;
            if(!list.contains(time))
                list.push_back(time);
        }
    }

    return list;
}
