#include <assert.h>
#include "cake.h"
#include "tensor.h"

crust::crust(LFLegaliser *legaliser, Tile *t, double tessCentreX, double tessCentreY): tile(t) {
    this->direction = calDirection(tessCentreX, tessCentreY);
    this->crowdIdx = calCrowdIdx(legaliser);
};

double crust::calDirection(double tessCentreX, double tessCentreY){
    Cord tileLL = this->tile->getLowerLeft();
    Cord tileUR = this->tile->getUpperRight();
    double tileCentreX = ((double)(tileLL.x + tileUR.x))/2;
    double tileCentreY = ((double)(tileLL.y + tileUR.y))/2;
    return tns::calVectorAngle(tessCentreX, tessCentreY, tileCentreX, tileCentreY);
};

double crust::calCrowdIdx(LFLegaliser *legaliser){
    double NeighborsOverlapArea = 0;

    std::vector <Tile *> neighbors;
    legaliser->findAllNeighbors(this->tile, neighbors);
    
    std::vector <Tessera *> surroundingTess;
    for(Tile *nb : neighbors){
        if(nb->getType() != tileType::BLANK){
            std::vector<Tessera *> inTess;
            legaliser->searchTesseraeIncludeTile(nb, inTess);
            for(Tessera* t : inTess){
                if(!checkVectorInclude(surroundingTess, t)){
                    surroundingTess.push_back(t);
                }
            }
        }
    }

    for(Tessera *nbTess : surroundingTess){
        for(Tile *t : nbTess->OverlapArr){
            NeighborsOverlapArea += (double)(t->getArea());
        }
    }

    return NeighborsOverlapArea / (double)(this->tile->getArea());

};

cake::cake(LFLegaliser *legaliser, Tile *overlap, int overlapLV){
    this->mLegaliser = legaliser;
    this->mOverlap = overlap;
    this->mOverlapLevel = overlapLV;

    assert(overlapLV >= 2);
    assert(overlapLV <= 4);
    
    mLegaliser->searchTesseraeIncludeTile(this->mOverlap, this->mMothers);

    assert(this->mMothers.size() == this->mOverlapLevel);

}

cake::~cake(){
    for(int i = 0; i < 4; ++i){
        for(crust *c : this->surroundings[i]){
            delete(c);
        }
    }
}

void cake::collectCrusts(LFLegaliser *legaliser){
    
    // there could be optimisation here: by preknowing the linkings, we may prioritize some blank tiles
    
    for(int i = 0; i < mMothers.size(); ++i){
        Tessera *tess = mMothers[i];
        Cord BBLL = tess->getBBLowerLeft();
        Cord BBUR = tess->getBBUpperRight();

        double tessCentreX = ((double)(BBLL.x + BBUR.x))/2;
        double tessCentreY = ((double)(BBLL.y + BBUR.y))/2;

        std::vector <Cord> record;
        for(Tile *tileInTess : tess->TileArr){
            std::vector <Tile *> Neighbors;
            mLegaliser->findAllNeighbors(tileInTess, Neighbors);
            for(Tile *t : Neighbors){
                bool seenBefore = checkVectorInclude(record, t->getLowerLeft());
                if((t->getType() == tileType::BLANK) && (!seenBefore)){
                    record.push_back(t->getLowerLeft());
                    crust *cr = new crust(legaliser, t, tessCentreX, tessCentreY);
                    this->surroundings[i].push_back(cr);
                }
            }

        }
        for(Tile *tileInTess : tess->OverlapArr){
            std::vector <Tile *> Neighbors;
            mLegaliser->findAllNeighbors(tileInTess, Neighbors);
            for(Tile *t : Neighbors){
                bool seenBefore = checkVectorInclude(record, t->getLowerLeft());
                if((t->getType() == tileType::BLANK) && (!seenBefore)){
                    record.push_back(t->getLowerLeft());
                    crust *cr = new crust(legaliser, t, tessCentreX, tessCentreY);
                    this->surroundings[i].push_back(cr);
                }
            }

        }
    }

}

void cake::showCake(){
    std::cout << "[C]";
    mOverlap->show(std::cout, true);
    std::cout <<"Mothers:" << std::endl;
    for(Tessera *tess : mMothers){
        std::cout << tess->getName() << " " << tess->getBBLowerLeft() << " " << tess->getBBUpperRight() << std::endl;
    }
    
    std::cout << std::endl << "Surroundings:" << std::endl;
    for(int i = 0; i <mMothers.size(); ++i){
        std::cout << "Mothers #" << i << std::endl;
        for(crust *c : surroundings[i]){
            std::cout << "(cr)";
            c->tile->show(std::cout, false);
            std::cout << ", Direction: " << c->direction << ", crowdIdx: " << c->crowdIdx << std::endl;
        }
    }
}



