//===-- LocalOpts.cpp - Example Transformations --------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Utils/LocalOpts.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstrTypes.h"

using namespace llvm;
//Funzione per stampare messaggio Algebrical Identity
void print_algebrical_identity(Instruction& i, Value *op, bool AddOrMul){
  if(AddOrMul == true){
    outs() << "ADD ALGEBRICAL IDENTITY\n";
  } else{
    outs() << "MUL ALGEBRICAL IDENTITY\n";
  }
  outs() << "\t     USES OF: " ;
  i.printAsOperand(outs(), false);
  outs() << "\n\tREPLACE WITH: ";
  op->printAsOperand(outs(), false);
  outs() << "\n";
}

//Funzione per stampare messaggio Strength Reductio 
void print_strength_reduction(Instruction& i, Instruction* is){
  outs() << "STRENGHT REDUCTION\n";
  outs() << "\t     USES OF: " << i << " -> ";
  i.printAsOperand(outs(), false);
  outs() << "\n\tREPLACE WITH: " << *is << " -> ";
  is->printAsOperand(outs(), false);
  outs() << "\n";
}

//Funzione per stamparemessaggio  Multi-Instruction Reduction
void print_multiInstruction_reduction(Instruction& i, Instruction* is, Value *op){
  outs() << "MULTI-INSTRUCTION REDUCTION: \n";
  outs() << "\t     USES OF: " << i << " -> ";
  i.printAsOperand(outs(), false);
  outs() << "   + " << *is << " -> ";
  is->printAsOperand(outs(), false);
  outs() << "\n\tREPLACE WITH:   " << *op << "\n";
}

//Funzione che implementa Algebrical Identity
void AlgebricIdentity(BasicBlock &B){
  outs() << "ENTRANDO NELLA FUNZIONE ALGEBRIC IDENTITY\n";
  Value *op1, *op2;
  for(Instruction &inst : B){
    //Add algebrical identity
    if(inst.getOpcode() == Instruction::Add){
      op1 = inst.getOperand(0);
      op2 = inst.getOperand(1);
      if(ConstantInt *C = dyn_cast<ConstantInt>(op1)){
        if(C->getValue()==0){
          print_algebrical_identity(inst, op2, true);
          inst.replaceAllUsesWith(op2);
        }
      }
      if(ConstantInt *C = dyn_cast<ConstantInt>(op2)){
        if(C->getValue()==0){
          print_algebrical_identity(inst, op1, true);
          inst.replaceAllUsesWith(op1);
        }
      }
    }
    //Mul algebrical identity
    if(inst.getOpcode() == Instruction::Mul){
      op1 = inst.getOperand(0);
      op2 = inst.getOperand(1);
      if(ConstantInt *C = dyn_cast<ConstantInt>(op1)){
        if(C->getValue()==1){
          print_algebrical_identity(inst, op2, false);
          inst.replaceAllUsesWith(op2);
        }
      }
      if(ConstantInt *C = dyn_cast<ConstantInt>(op2)){
        if(C->getValue()==1){
          print_algebrical_identity(inst, op1, false);
          inst.replaceAllUsesWith(op1);
        }
      }
    }

  }
  outs() << "USCENDO DALLA FUNZIONE ALGEBRIC IDENTITY\n\n";
}

//Funzione che implementa Strenght Reduction
void StrengthReduction(BasicBlock &B){
  outs() << "ENTRANDO NELLA FUNZIONE STRENGTH REDUCTION\n";
  Value *op1, *op2;
  for(Instruction &inst : B){
    op1 = inst.getOperand(0);
    op2 = inst.getOperand(1);
    if(inst.getOpcode() == Instruction::Mul){
      if(ConstantInt *C = dyn_cast<ConstantInt>(op1)){
        if(C->getValue().isPowerOf2()){
          auto numShift = C->getValue().logBase2();
          Constant *C1 = ConstantInt::get(C->getType(), numShift);
          Instruction *shiftInst = BinaryOperator::CreateShl(op2, C1);

          shiftInst->insertAfter(&inst);
          inst.replaceAllUsesWith(shiftInst);

          outs() << "MUL ";
          print_strength_reduction(inst, shiftInst);
        }
        else{
          if(C->getValue().ugt(1)){
            APInt n = C->getValue()+1;
            if(n.isPowerOf2()){
              auto numShift = n.logBase2();
              Constant *C1 = ConstantInt::get(C->getType(), numShift);
              Instruction *shiftInst = BinaryOperator::CreateShl(op2, C1);
              Instruction *difInst = BinaryOperator::CreateSub(shiftInst, op2);

              shiftInst->insertAfter(&inst);
              difInst->insertAfter(shiftInst);
              inst.replaceAllUsesWith(difInst);

              outs() << "MUL +1 ";
              print_strength_reduction(inst, difInst);
            }
          }
        }
      }
      if(ConstantInt *C = dyn_cast<ConstantInt>(op2)){
        if(C->getValue().isPowerOf2()){
          auto numShift = C->getValue().logBase2();
          Constant *C1 = ConstantInt::get(C->getType(), numShift);
          Instruction *shiftInst = BinaryOperator::CreateShl(op1, C1);

          shiftInst->insertAfter(&inst);
          inst.replaceAllUsesWith(shiftInst);

          outs() << "MUL ";
          print_strength_reduction(inst, shiftInst);
        }
        else{
          if(C->getValue().ugt(1)){
            APInt n = C->getValue()+1;
            if(n.isPowerOf2()){
              auto numShift = n.logBase2();
              Constant *C1 = ConstantInt::get(C->getType(), numShift);
              Instruction *shiftInst = BinaryOperator::CreateShl(op1, C1);
              Instruction *difInst = BinaryOperator::CreateSub(shiftInst, op1);

              shiftInst->insertAfter(&inst);
              difInst->insertAfter(shiftInst);
              inst.replaceAllUsesWith(difInst);
              
              outs() << "MUL +1 ";
              print_strength_reduction(inst, difInst);
            }
          }
        }
      }
    }
    else{
      if(inst.getOpcode() == Instruction::SDiv){
        if(ConstantInt *C = dyn_cast<ConstantInt>(op1)){
          if(C->getValue().isPowerOf2()){
            auto numShift = C->getValue().logBase2();
            Constant *C1 = ConstantInt::get(C->getType(), numShift);

            Instruction *shiftInst = BinaryOperator::CreateAShr(op2, C1);
            shiftInst->insertAfter(&inst);
            inst.replaceAllUsesWith(shiftInst);

            outs() << "DIV ";
            print_strength_reduction(inst, shiftInst);
          }
        }
        else{
          if(ConstantInt *C = dyn_cast<ConstantInt>(op2)){
            if(C->getValue().isPowerOf2()){
              
              auto numShift = C->getValue().logBase2();
              Constant *C1 = ConstantInt::get(C->getType(), numShift);

              Instruction *shiftInst = BinaryOperator::CreateAShr(op1, C1);
              shiftInst->insertAfter(&inst);
              inst.replaceAllUsesWith(shiftInst);

              outs() << "DIV ";
              print_strength_reduction(inst, shiftInst);
            }
          }
        }
      }
    }
  }
  outs() << "USCENDO DALLA FUNZIONE STRENGTH REDUCTION\n\n";
}

//Funzione che implementa Multi-Instruction Reduction
void MultiInstructionReduction(BasicBlock &B){
  outs() << "ENTRANDO NELLA FUNZIONE MULTI-INSTRUCTION REDUCTION\n";
  Value *op1, *op2;
  for(Instruction &inst : B){
    op1 = inst.getOperand(0);
    op2 = inst.getOperand(1);
    //MultiInstructionReduction add before sub
    if(inst.getOpcode()==Instruction::Add){
      if(ConstantInt* C = dyn_cast<ConstantInt>(op1)){
        for(auto it = inst.user_begin(); it != inst.user_end(); ++it){
          Instruction *instUser = dyn_cast<Instruction>(*it);
          Value *op1User = instUser->getOperand(0);
          Value *op2User = instUser->getOperand(1);
          if(instUser->getOpcode() == Instruction::Sub){
            if(ConstantInt* C1 = dyn_cast<ConstantInt>(op1User)){
              if(C->getValue().eq(C1->getValue())){
                instUser->replaceAllUsesWith(op2);

                print_multiInstruction_reduction(inst, instUser, op2);
              }
            }
            else{
              if((C1 = dyn_cast<ConstantInt>(op2User))){
                if(C->getValue().eq(C1->getValue())){
                  instUser->replaceAllUsesWith(op2);

                  print_multiInstruction_reduction(inst, instUser, op2);
                }
              }
            }
          }
        }
      }
      else{
        if(ConstantInt* C = dyn_cast<ConstantInt>(op2)){
          for(auto it = inst.user_begin(); it != inst.user_end(); ++it){
            Instruction *instUser = dyn_cast<Instruction>(*it);
            Value *op1User = instUser->getOperand(0);
            Value *op2User = instUser->getOperand(1);
            if(instUser->getOpcode() == Instruction::Sub){
              if(ConstantInt* C1 = dyn_cast<ConstantInt>(op1User)){
                if(C->getValue().eq(C1->getValue())){
                  instUser->replaceAllUsesWith(op1);

                  print_multiInstruction_reduction(inst, instUser, op1);
                }
              }
              else{
                if((C1 = dyn_cast<ConstantInt>(op2User))){
                  if(C->getValue().eq(C1->getValue())){
                    instUser->replaceAllUsesWith(op1);

                    print_multiInstruction_reduction(inst, instUser, op1);
                  }
                }
              }
            }
          }
        }
      }
    }
    else{
      if(inst.getOpcode()==Instruction::Sub){
        if(ConstantInt* C = dyn_cast<ConstantInt>(op1)){
          for(auto it = inst.user_begin(); it != inst.user_end(); ++it){
            Instruction *instUser = dyn_cast<Instruction>(*it);
            Value *op1User = instUser->getOperand(0);
            Value *op2User = instUser->getOperand(1);
            if(instUser->getOpcode() == Instruction::Add){
              if(ConstantInt* C1 = dyn_cast<ConstantInt>(op1User)){
                if(C->getValue().eq(C1->getValue())){
                  instUser->replaceAllUsesWith(op2);

                  print_multiInstruction_reduction(inst, instUser, op2);
                }
              }
              else{
                if((C1 = dyn_cast<ConstantInt>(op2User))){
                  if(C->getValue().eq(C1->getValue())){
                    instUser->replaceAllUsesWith(op2);

                    print_multiInstruction_reduction(inst, instUser, op2);
                  }
                }
              }
            }
          }
        }
        else{
          if(ConstantInt* C = dyn_cast<ConstantInt>(op2)){
            for(auto it = inst.user_begin(); it != inst.user_end(); ++it){
              Instruction *instUser = dyn_cast<Instruction>(*it);
              Value *op1User = instUser->getOperand(0);
              Value *op2User = instUser->getOperand(1);
              if(instUser->getOpcode() == Instruction::Add){
                if(ConstantInt* C1 = dyn_cast<ConstantInt>(op1User)){
                  if(C->getValue().eq(C1->getValue())){
                    instUser->replaceAllUsesWith(op1);

                    print_multiInstruction_reduction(inst, instUser, op1);
                  }
                }
                else{
                  if((C1 = dyn_cast<ConstantInt>(op2User))){
                    if(C->getValue().eq(C1->getValue())){
                      instUser->replaceAllUsesWith(op1);

                      print_multiInstruction_reduction(inst, instUser, op1);
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  outs() << "USCENDO DALLA FUNZIONE MULTI-INSTRUCTION REDUCTION\n\n";
}

bool runOnBasicBlock(BasicBlock &B) {

  AlgebricIdentity(B);
  StrengthReduction(B);
  MultiInstructionReduction(B);

  // Preleviamo le prime due istruzioni del BB
  Instruction &Inst1st = *B.begin(), &Inst2nd = *(++B.begin());

  // L'indirizzo della prima istruzione deve essere uguale a quello del 
  // primo operando della seconda istruzione (per costruzione dell'esempio)
  assert(&Inst1st == Inst2nd.getOperand(0));
  outs() << "\n\n---------------------------------------\n\n";
  // Stampa la prima istruzione
  outs() << "PRIMA ISTRUZIONE: " << Inst1st << "\n";
  // Stampa la prima istruzione come operando
  outs() << "COME OPERANDO: ";
  Inst1st.printAsOperand(outs(), false);
  outs() << "\n";

  // User-->Use-->Value
  outs() << "I MIEI OPERANDI SONO:\n";
  for (auto *Iter = Inst1st.op_begin(); Iter != Inst1st.op_end(); ++Iter) {
    Value *Operand = *Iter;

    if (Argument *Arg = dyn_cast<Argument>(Operand)) {
      outs() << "\t" << *Arg << ": SONO L'ARGOMENTO N. " << Arg->getArgNo() 
        <<" DELLA FUNZIONE " << Arg->getParent()->getName()
              << "\n";
    }
    if (ConstantInt *C = dyn_cast<ConstantInt>(Operand)) {
      outs() << "\t" << *C << ": SONO UNA COSTANTE INTERA DI VALORE " << C->getValue()
              << "\n" ;
      C->getType()->print(outs(), false, true);
    }
  }

  outs() << "LA LISTA DEI MIEI USERS:\n";
  for (auto Iter = Inst1st.user_begin(); Iter != Inst1st.user_end(); ++Iter) {
    outs() << "\t" << *(dyn_cast<Instruction>(*Iter)) << "\n";
  }

  outs() << "E DEI MIEI USI (CHE E' LA STESSA):\n";
  for (auto Iter = Inst1st.use_begin(); Iter != Inst1st.use_end(); ++Iter) {
    outs() << "\t" << *(dyn_cast<Instruction>(Iter->getUser())) << "\n";
  }

  // Manipolazione delle istruzioni
  Instruction *NewInst = BinaryOperator::Create(
      Instruction::Add, Inst1st.getOperand(0), Inst1st.getOperand(0));

  NewInst->insertAfter(&Inst1st);
  // Si possono aggiornare le singole references separatamente?
  // Controlla la documentazione e prova a rispondere.
  Inst1st.replaceAllUsesWith(NewInst);

  return true;
}


bool runOnFunction(Function &F) {
  bool Transformed = false;
  for (auto Iter = F.begin(); Iter != F.end(); ++Iter) {
    if (runOnBasicBlock(*Iter)) {
      Transformed = true;
    }
  }

  return Transformed;
}


PreservedAnalyses LocalOpts::run(Module &M, ModuleAnalysisManager &AM) {
  for (auto Fiter = M.begin(); Fiter != M.end(); ++Fiter)
    if (runOnFunction(*Fiter))
      return PreservedAnalyses::none();
  
  return PreservedAnalyses::all();
}
