# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## 이 프로젝트는 무엇인가

"반도체 시료 생산주문관리 시스템" 콘솔 애플리케이션을 개발하는프로젝트입니다.


## 작업 지침
사용자 요구사항은 docs 폴더 아래에 위치한다.
신규 기능 추가 작업은 plan 을 기반으로 한다.

코드를 작성하거나 수정할 때는 [dev/coding-convention.md](dev/coding-convention.md)에 정의된 코딩 컨벤션을 따른다.

## tdd-workflow 실행 규칙

`.claude/workflows/tdd-workflow.js`는 `args.step`으로 지정된 단계(plan/red/green/refactor) 하나만 실행하고 즉시 반환한다. 단계를 이어서 자동 실행하지 않는다.
tdd-workflow를 호출하는 쪽(메인 대화 루프)은 매 호출 후 반드시 개발자에게 결과를 보여주고 명시적 확인(AskUserQuestion 등)을 받은 뒤에만 다음 단계를 호출해야 한다. 한 턴 안에서 확인 없이 여러 단계를 연달아 호출하지 않는다.
