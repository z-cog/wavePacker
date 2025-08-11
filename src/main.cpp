#include <midifile/midifile.h>
#include <wave/chunk.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#include <stdint.h>

// int LOWBOUND = 31;
// int MIDBOUND = 63;
// int HIGHBOUND = 95;

int DEBUG = 0;

std::string getNoteRange(int note){
    note = note % 4;
    if(note == 0){
        return "LEFT";
    }else if(note == 1){
        return "DOWN";
    }else if(note == 2){
        return "UP";
    }else {
        return "RIGHT";
    }
}



int parseMidi(std::string fileName, std::vector<double>* timeStamps, std::vector<std::string>* cueLabels){
    smf::MidiFile currentMidi;
    if(!currentMidi.read(fileName + ".mid")){
        std::cerr << "Unable to read MIDI file: " << (fileName + ".mid") << std::endl;
        return 1;
    }

    std::cout << "read file: " << fileName << std::endl;

    currentMidi.doTimeAnalysis();

    // This could be used to check for both on and off events. We're not gonna bother tho.
    // currentMidi.linkNotePairs();
    std::ofstream cueFile;
    
    if(DEBUG){
        cueFile.open(fileName + "Cues.txt");
        if(!cueFile.is_open()) {
            std::cerr << "Unable to open cue file" << std::endl;
            return 1;
        }
    }

    for(int track = 0; track < currentMidi.getTrackCount(); ++track) {
        if(DEBUG){
            std::cout << "Current track: " << track << std::endl;
        }
        for(int i = 0; i < currentMidi[track].getEventCount(); ++i){
            smf::MidiEvent& event = currentMidi.getEvent(track, i);

            if(DEBUG){
                std::cout << "Event  " << i << ": ";
                if(!event.isEmpty()){
                    if(event.isNote()){
                        std::cout << (event.isNoteOn() ? "NoteOn " : "")
                        << "key=" << event.getKeyNumber() << " "
                        << "vel=" << event.getVelocity() << " "
                        << "sec=" << event.getDurationInSeconds() << " "
                        << std::endl;
                    }else{
                        std::cout << "not a note" << std::endl;
                    }
                }else{
                    std::cout << "null" << std::endl;
                }
            }

            if(event.isNoteOn() && event.getVelocity() > 0.0001) {
                double time = event.seconds;
                std::string label = getNoteRange(event.getKeyNumber());
                if(DEBUG){
                    cueFile << time << "\t" << label << "\n";
                    std::cout << "=> " << time << "\t" << label << std::endl;

                    std::cout << "     note: " << event.getKeyNumber() << std::endl;
                }
                
                (*timeStamps).emplace_back(time);
                (*cueLabels).emplace_back(label);
            }
        }
    }

    if(DEBUG){
        cueFile << std::endl;
        cueFile.close();
        std::cout << std::endl;
    }

    return 0;
}



int populateWave(std::string fileName, std::vector<double> timeStamps, std::vector<std::string> cueLabels){

    char pad = 0;

    std::ifstream src(fileName + ".wav", std::ios::binary);
    
    std::ofstream dst("A_ " + fileName + "WithCues.wav", std::ios::binary);

    if (!src) {
        std::cerr << "Unable to open source WAVE file" << std::endl;
        return 1;
    }

    if (!dst) {
        std::cerr << "Unable to open output WAVE file" << std::endl;
        return 1;
    }

    //////////////////////
    //// RIFF AND FMT ////
    //////////////////////
    
    uint32_t sampleRate = 44100;
    std::streampos insertionPoint = -1;
    src.seekg(12, std::ios::beg);

    char currChunk[4];
    uint32_t chunkSize;

    while(src.read(currChunk, 4) && src.read(reinterpret_cast<char *>(&chunkSize), 4)){
        std::streampos chunkDataStart = src.tellg();
        
        if(std::strncmp(currChunk, "fmt ", 4) == 0) {
            insertionPoint = chunkDataStart;
            src.seekg(4, std::ios::cur);
            src.read(reinterpret_cast<char*>(&sampleRate), 4);

            insertionPoint = chunkDataStart + static_cast<std::streamoff>(chunkSize + (chunkSize % 2)); 
            break;
        }

        src.seekg(chunkDataStart + static_cast<std::streamoff>(chunkSize + (chunkSize % 2)));
    }


    if(insertionPoint < 0){
        std::cerr << "Unable to locate DATA chunk!" << std::endl;
        return 1;
    }

    std::vector<char> fileCopyBuffer(insertionPoint);
    src.seekg(0);
    src.read(fileCopyBuffer.data(), insertionPoint);
    dst.write(fileCopyBuffer.data(), insertionPoint);

    if(DEBUG){
        std::cout << "File DIFF and fmt chunk copied." << std::endl;
        std::cout << "sampleRate: " << sampleRate << std::endl;
    }

    //////////////////////
    //////// CUES ////////
    //////////////////////

    Chunk cueHeader;
    cueHeader.ckID = CHUNK_ID_CUE;
    cueHeader.ckSize = 4 + timeStamps.size()*sizeof(CuePoint);
    cueHeader.ckInfo = timeStamps.size();
    
    dst.seekp(0, std::ios::end);
    dst.write(reinterpret_cast<char*>(&cueHeader), sizeof(cueHeader));

    if(DEBUG){
        std::cout << "cueHeader written:\n     ckID: " << cueHeader.ckID << "\n     size: " << cueHeader.ckSize << "\n     numCues: " << cueHeader.ckInfo << std::endl;
    }

    for(int i = 0; i < timeStamps.size(); i++){
        CuePoint currentCue;
        currentCue.ID = static_cast<uint32_t>(i);
        currentCue.position = static_cast<uint32_t>(timeStamps[i] * sampleRate);
        currentCue.dataChunkID = CHUNK_ID_DATA;
        currentCue.chunkStart = 0;
        currentCue.blockStart = 0;
        currentCue.sampleOffset = static_cast<uint32_t>(timeStamps[i] * sampleRate);

        dst.write(reinterpret_cast<char*>(&currentCue), sizeof(currentCue));

        if(DEBUG){
            std::cout << "cuePoint written:\n     ID: " << currentCue.ID << "\n     time: " << currentCue.position << std::endl;
        }
    }
    
    if(cueHeader.ckSize % 2 != 0){
        dst.write(&pad, 1);
    }

    //////////////////////
    /////// LABELS ///////
    //////////////////////

    Chunk listHeader;
    listHeader.ckID = CHUNK_ID_LIST;
    listHeader.ckSize = 4 + (8 * cueLabels.size()); //temp size, just in case
    listHeader.ckInfo = LIST_TYPE_ADTL;

    std::streampos currentPlace = dst.tellp();
    dst.seekp(0, std::ios::end);
    
    for(int i = 0; i < (static_cast<uint32_t>(currentPlace)%4); i++){
        dst.write(&pad, 1);
        if(DEBUG){
            std::cout << "Wrote " << i << "pad bytes before ADTL chunk." << std::endl;
        }
    }

    dst.seekp(0, std::ios::end);
    dst.write(reinterpret_cast<char*>(&listHeader), sizeof(listHeader));
    
    if(DEBUG){
        std::cout << "Wrote LIST header\n     ckID: " << listHeader.ckID << "\n     size: " << listHeader.ckSize << "\n     listType: " << listHeader.ckInfo << std::endl;
    }

    currentPlace = dst.tellp();
    
    if(DEBUG){
        std::cout << "seeking to position: " << currentPlace << std::endl;
    }

    uint32_t startPlace = static_cast<uint32_t>(currentPlace);
    int nullFlag;
    for(int i = 0; i < cueLabels.size(); i++){
        nullFlag = 0;

        Chunk currentLabelHeader;
        currentLabelHeader.ckID = CHUNK_ID_LABL;
        currentLabelHeader.ckInfo = static_cast<uint32_t>(i);
        
        std::string currentLabel = cueLabels[i];
        
        if(currentLabel.size() > 63) {
            currentLabel = currentLabel.substr(0, 63);
        }

        uint32_t labelSize = static_cast<uint32_t>(currentLabel.length()) + 1;

        if(labelSize % 2 != 0){
            nullFlag = 1;
        }
        
        currentLabelHeader.ckSize = 4 + labelSize + nullFlag;

        if(DEBUG){
            std::cout << "labelSize: " << labelSize << std::endl;
        }
        
        dst.write(reinterpret_cast<char*>(&currentLabelHeader), sizeof(currentLabelHeader));

        
        
        if(DEBUG){
            std::cout << "Wrote LABEL header\n     ckID: " << currentLabelHeader.ckID << "\n     size: " << currentLabelHeader.ckSize << "\n     subID: " << currentLabelHeader.ckInfo << std::endl;
        }
        
        dst.write(currentLabel.c_str(), labelSize);
        
        if(nullFlag){
            dst.write(&pad, 1);            
        }
        
        if(DEBUG){
            std::cout << "Wrote LABEL: " << currentLabel << std::endl;
        }
    }
    
    currentPlace = dst.tellp();
    uint32_t endPlace = static_cast<uint32_t>(currentPlace);

    uint32_t updatedSize = static_cast<uint32_t>(endPlace) - static_cast<uint32_t>(startPlace) + 4; 

    dst.seekp(-(static_cast<int>(updatedSize) + 4), std::ios::end);
    
    dst.write(reinterpret_cast<char*>(&updatedSize), sizeof(updatedSize));

    if(listHeader.ckSize % 2 != 0){
        dst.write(&pad, 1);
    }

    if(DEBUG){
        std::cout << "Updated labelHeader size to: " << updatedSize << std::endl;
    }

    //////////////////////
    //////// DATA ////////
    //////////////////////

    src.seekg(insertionPoint);
    dst.seekp(0, std::ios::end);
    fileCopyBuffer.clear();
    fileCopyBuffer.insert(fileCopyBuffer.end(), std::istreambuf_iterator<char>(src),std::istreambuf_iterator<char>());

    dst.write(fileCopyBuffer.data(), fileCopyBuffer.size());

    if(DEBUG){
        std::cout << "Original data chunk inserted." << std::endl;
    }

    src.close();

    //////////////////////
    // UPDATE RIFF SIZE //
    //////////////////////
    
    dst.seekp(0, std::ios::end);
    currentPlace = dst.tellp();
    updatedSize = static_cast<uint32_t>(currentPlace) - 8;

    dst.seekp(4, std::ios::beg);
    dst.write(reinterpret_cast<char*>(&updatedSize), sizeof(updatedSize));

    dst.close();

    std::cout << "File written.\n" << "Final Size: " << updatedSize + 8 << std::endl;
    return 0;

}

int displayHelp(std::string input){
    std::cout <<
        "Usage: wavePacker [--debug|-d] input\n\n" <<
        "wavePacker takes in a MIDI file (filename.mid) and converts it into labeled cue points " <<
        "which are then inserted into a copy of a WAVE file (filename.wav).\n\n" <<
        "Both files must begin with the same handle, and must be in the same directory as this " <<
        "program.\n\n" <<
        "Press \"Enter\" to close this prompt."
        << std::endl;
        std::getline(std::cin, input);
        return 0;
}



int main(int argc, char** argv) {

    std::string input;
    
    if(argc >= 2 && ((strcmp(argv[1], "--help") == 0) || (strcmp(argv[1], "-h") == 0))){
        displayHelp(input);
        return 0;
    }
    
    std::string fileName;

    if(argc == 1){

        std::cout << "Display help? [yes|no]: ";
        std::getline(std::cin, input);
        std::cout << std::endl;

        if((input == "yes") || (input == "y") || (input == "Y") || (input == "Yes")){
            displayHelp(input);
            return 0;
        }

        std::cout << "Enable debug mode? [yes|no]: ";
        std::getline(std::cin, input);
        std::cout << std::endl;

        if((input == "yes") || (input == "y") || (input == "Y") || (input == "Yes")){
            DEBUG = 1;
        }

        std::cout << "Enter filename: ";
        std::getline(std::cin, input);
        std::cout << std::endl;
        
        if(input.length() <= 0){
            std::cerr << "filename is required." << std::endl;
            return 1;
        }else{
            fileName = input;
        }

        // std::cout << "Choose low bound (0-127, default=31): ";
        // std::getline(std::cin, input);
        // std::cout << std::endl;

        // if(input.length() > 0){
        //     LOWBOUND = std::stoi(input);
        // }

        // std::cout << "Choose mid bound (0-127, default=63): ";
        // std::getline(std::cin, input);
        // std::cout << std::endl;
        
        // if(input.length() > 0){
        //     MIDBOUND = std::stoi(input);
        // }
        
        // std::cout << "Choose high bound (0-127, default=95): ";
        // std::getline(std::cin, input);
        // std::cout << std::endl;

        // if(input.length() > 0){
        //     HIGHBOUND = std::stoi(input);
        // }

    } else {
        std::cout << "argc = " << argc <<std::endl;
        if((argc < 2)) {
            std::cerr << "Usage: " << argv[0] << "wavePacker [--debug|-d] input" << std::endl;
            std::cerr << "Use  '[--help|-h]' for more details" << std::endl;
            return 1;
        }
        
        std::cout << argv[1] << std::endl;
        if((strcmp(argv[1], "--debug") == 0) || (strcmp(argv[1], "-d") == 0)){
            DEBUG = 1;
        }

        // if(argc > 2+DEBUG){
        //     LOWBOUND = std::stoi(argv[2+DEBUG]);
        //     if(argc > 3+DEBUG){
        //         MIDBOUND = std::stoi(argv[3+DEBUG]);
        //         if(argc > 4+DEBUG){
        //             HIGHBOUND = std::stoi(argv[4+DEBUG]);
        //         }
        //     }
        // }
        
        if(argc >= 2+DEBUG){
            fileName = argv[1+DEBUG];
        }else{
            std::cerr << "filename is required." << std::endl;
            return 1;
        }
    }

    if(DEBUG) {
        std::cout << "Beginning Debug Output" <<std::endl;
        std::cout << std::endl;
    }
    
    if(DEBUG){
        std::cout << "filename: " << fileName << std::endl;

        // std::cout << "Up = 0-"  << LOWBOUND << std::endl;
        // std::cout << "Down = "  << LOWBOUND+1 << "-" << MIDBOUND << std::endl;
        // std::cout << "Left = "  << MIDBOUND+1 << "-" << HIGHBOUND << std::endl;
        // std::cout << "Right = " << HIGHBOUND+1 << "-127" << std::endl << std::endl;
    }

    std::vector<double> timeStamps;
    std::vector<std::string> cueLabels;

    if(parseMidi(fileName, &timeStamps, &cueLabels)){
        std::cerr << "parseMidi unable to be completed!" << std::endl;
        return 1;
    }

    if(DEBUG){
        std::cout << "timeStamps Size: " << timeStamps.size() << std::endl;
        std::cout << "cueLabels Size: " << timeStamps.size() << std::endl << std::endl;
    }

    if(populateWave(fileName, timeStamps, cueLabels)){
        std::cerr << "populateWave unable to be completed!" << std::endl;
        return 1;
    }

    if(DEBUG){
        std::cout <<
        "\n\nPress \"Enter\" to close this terminal."
        << std::endl;
        std::getline(std::cin, input);
    }
    return 0;
}
