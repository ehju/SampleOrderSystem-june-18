export const meta = {
  name: 'tdd-workflow',
  description: 'Execute exactly one TDD step (plan | red | green | refactor) and return — the caller must pause for explicit developer confirmation before invoking the next step',
  whenToUse: 'When the user wants a coding requirement implemented via strict TDD with gmock, phase by phase. IMPORTANT: this workflow performs ONLY the single step named in args.step and then returns immediately. It never advances to the next step on its own — there is no in-script mechanism to pause for human input. Whoever invokes this workflow (the main conversation loop) MUST stop after each call, show the result to the developer, and get explicit confirmation (e.g. via AskUserQuestion) before calling this workflow again for the next step. Never chain multiple steps/invocations within the same turn without that confirmation.',
  phases: [
    { title: 'Plan', detail: 'read requirement, write PLAN.md and phase{n}.md files' },
    { title: 'RED', detail: 'write failing gmock tests from the phase spec only' },
    { title: 'GREEN', detail: 'make the tests pass' },
    { title: 'REFACTOR', detail: 'review against spec, refactor, decide if phase is done' },
  ],
}

const PLAN_SCHEMA = {
  type: 'object',
  properties: {
    phaseCount: { type: 'number', description: 'total number of phases created (phase1.md..phaseN.md)' },
    summary: { type: 'string', description: 'one-line summary of the phase breakdown' },
  },
  required: ['phaseCount'],
}

const REFACTOR_VERDICT_SCHEMA = {
  type: 'object',
  properties: {
    requirementsMet: { type: 'boolean', description: 'are all requirements in phase{n}.md fully implemented' },
    refactorDone: { type: 'boolean', description: 'is the resulting code clean, with no further refactor needed' },
    notes: { type: 'string', description: 'what is missing, or what was changed during refactor' },
  },
  required: ['requirementsMet', 'refactorDone'],
}

const VALID_STEPS = ['plan', 'red', 'green', 'refactor']

if (!args || !VALID_STEPS.includes(args.step)) {
  throw new Error(`args.step 은 다음 중 하나여야 한다: ${VALID_STEPS.join(', ')}`)
}

if (args.step === 'plan') {
  if (!args.requirements) {
    throw new Error('args.step === "plan" 이면 args.requirements (string) 가 필요하다 — 예: docs/specs 요구사항 파일 내용')
  }

  phase('Plan')
  const plan = await agent(
    `다음 요구사항을 구현해야 한다:

${args.requirements}

이 요구사항을 phase 단위로 쪼개 PLAN.md와 phase{N}.md 파일들을 작성하라.`,
    { schema: PLAN_SCHEMA, label: 'plan', agentType: 'tdd-planner' }
  )

  log(`계획 수립 완료: 총 ${plan.phaseCount}개 phase${plan.summary ? ' — ' + plan.summary : ''}`)
  log('다음 단계를 진행하기 전에 개발자 확인을 받으세요 (이 워크플로우는 자동으로 이어지지 않습니다).')
  return plan
}

if (!args.phaseNumber) {
  throw new Error(`args.step === "${args.step}" 이면 args.phaseNumber (number) 가 필요하다`)
}
const n = args.phaseNumber
const round = args.round || 1

if (args.step === 'red') {
  phase('RED')
  await agent(
    `phase${n}.md 의 요구사항에 대해 RED 단계를 수행하라 (라운드 ${round}).`,
    { label: `red-p${n}-r${round}`, phase: 'RED', agentType: 'tdd-red' }
  )
  log(`Phase ${n} RED(라운드 ${round}) 완료. 다음 단계(GREEN)로 넘어가기 전에 개발자 확인을 받으세요.`)
  return { phaseNumber: n, round, step: 'red', done: true }
}

if (args.step === 'green') {
  phase('GREEN')
  await agent(
    `phase${n}.md 의 요구사항에 대해 GREEN 단계를 수행하라 (라운드 ${round}).`,
    { label: `green-p${n}-r${round}`, phase: 'GREEN', agentType: 'tdd-green' }
  )
  log(`Phase ${n} GREEN(라운드 ${round}) 완료. 다음 단계(REFACTOR)로 넘어가기 전에 개발자 확인을 받으세요.`)
  return { phaseNumber: n, round, step: 'green', done: true }
}

// args.step === 'refactor'
phase('REFACTOR')
const verdict = await agent(
  `phase${n}.md 의 요구사항에 대해 REFACTOR 단계를 수행하라 (라운드 ${round}).`,
  { schema: REFACTOR_VERDICT_SCHEMA, label: `refactor-p${n}-r${round}`, agentType: 'tdd-refactor' }
)

log(`Phase ${n} REFACTOR(라운드 ${round}): requirementsMet=${verdict.requirementsMet}, refactorDone=${verdict.refactorDone}${verdict.notes ? ' — ' + verdict.notes : ''}`)

if (verdict.requirementsMet && verdict.refactorDone) {
  log(`Phase ${n} 완료. 다음 phase의 RED로 넘어가기 전에 개발자 확인을 받으세요 (모든 phase가 끝났다면 여기서 종료).`)
} else {
  log(`Phase ${n} 이 아직 완료되지 않았습니다. 같은 phase의 RED를 다음 라운드(${round + 1})로 다시 실행할지 개발자에게 확인받으세요.`)
}

return { phaseNumber: n, round, step: 'refactor', verdict }
